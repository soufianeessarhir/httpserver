/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 18:08:39 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/05 16:33:14 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/HttpServer.hpp"



HttpServer::HttpServer(std::vector<Server> &srvs):servers(srvs)
{
	epoll_fd = epoll_create1(0);
	SetSocketToNonblocking(epoll_fd);
	if (epoll_fd == -1)
		throw HttpServerError("Epoll creation failed");
	this->init();
}


void		HttpServer::init()
{
	struct addrinfo hints, *res, *p;
	
	for (size_t i = 0; i < servers.size(); ++i)
	{
		for (size_t j = 0; j < servers[i].listen.size(); ++j)
		{
			int sockfd = -1;
			memset(&hints, 0, sizeof(hints));
			hints.ai_family = AF_INET;   // [sessarhi] mybe should modefied to AF_UNSPEC for handling both ipv4 and ipv6
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_flags = AI_PASSIVE;
			std::string host = servers[i].listen[j].first.empty() ?
				"localhost" : servers[i].listen[j].first;
			std::ostringstream port;
			port << servers[i].listen[j].second;
			if (getaddrinfo(host.c_str(), port.str().c_str(),
				&hints, &res) != 0)
			{
				close(sockfd);
				throw HttpServerError("Getaddrinfo failed");
			}
			for (p = res; p != NULL; p = p->ai_next)
			{
				sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
				if (sockfd == -1)
					continue;
				int optval = 1;
				setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
				SetSocketToNonblocking(sockfd);
				if (bind(sockfd, p->ai_addr, p->ai_addrlen)!= -1)
					break;
				if (sockfd != -1)
					close(sockfd);
				sockfd = -1;
			}
			freeaddrinfo(res);
			if (p == NULL)
				throw HttpServerError("Socket binding failed");
			if (listen(sockfd, SOMAXCONN) == -1)
			{
				close(sockfd);
				throw HttpServerError("Socket listening failed");
			}
			ev.events = EPOLLIN | EPOLLET;
			ev.data.fd = sockfd;
			if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &ev) == -1)
			{
				close(sockfd);
				throw HttpServerError("Epoll control failed");
			}
			server_map[sockfd] = servers[i];
		}
	}
}

void		HttpServer::SetSocketToNonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw HttpServerError("fcntl get failed");
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw HttpServerError("fcntl set failed");
}

void		HttpServer::HandleNewConnection(int fd)
{
	// std::cout<< "accepted\n";
	for (;;)
	{
		struct sockaddr_storage client_sock;
		socklen_t socklen = sizeof(client_sock);
		int client_fd = accept(fd, (sockaddr *)&client_sock, &socklen);
		if (client_fd == -1)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK)
			{
				break;
			}
			else
				throw HttpServerError("Accept failed");
		}
		SetSocketToNonblocking(client_fd);
		Connection *conn = new Connection(client_fd);
		clients[client_fd] = conn;
		ev.events = EPOLLIN | EPOLLET;
		ev.data.fd = client_fd;
		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1)
			throw HttpServerError("Epoll control failed");
	}
}

void		HttpServer::ProcessRequestLine(Connection *conn)
{
	size_t end = conn->buffer.find("\r\n");
	if (end  != std::string::npos)
	{
		std::string line = conn->buffer.substr(0,end);
		if (line.empty())
		{
			conn->buffer.erase(0,end + 2);
			return ;
		}
		bool IsValid = conn->request->ParseRequestLine(line);
		if (IsValid)
		{
			conn->buffer.erase(0,end + 2);
			// std::cout<<conn->buffer<<std::endl;
			conn->state = Connection::READING_HEADERS;
			std::cout <<"reach file "<<__FILE__<<" line "<<__LINE__<<std::endl;
		}
		else
		{
			std::cout <<"reach file "<<__FILE__<<" line "<<__LINE__<<std::endl;
			conn->response =  new Response(conn->request->GetStatus());
			conn->state =  Connection::SENDING_RESPONSE;
		}
	}
}

void		HttpServer::ProcessHeaders(Connection *conn)
{
		size_t end = conn->buffer.find("\r\n\r\n");
			if (end != std::string::npos)
			{
				bool IsValid = conn->request->ParseHeaders(conn->buffer.substr(0,end + 4));
				if (IsValid)
				{
					conn->buffer.erase(0,end + 4);
					if (conn->request->GetIsComplet())
						conn->state = Connection::PROCESSING;
				}
				else
				{
					conn->response = new Response(conn->request->GetStatus());
					conn->state = Connection::SENDING_RESPONSE;
				}
			}
			else
			{
				size_t end = conn->buffer.find("\r\n");
				if (end != std::string::npos)
				{
					bool IsValid = conn->request->ParseHeaders(conn->buffer.substr(0,end + 2));
					if (IsValid)
						conn->buffer.erase(0,end + 2);
					else 
					{
						conn->response = new Response(conn->request->GetStatus());
						conn->state =  Connection::SENDING_RESPONSE;
					}
				}
			}
}

void		HttpServer::HandlIncommingData(int fd)
{
	Connection *conn = clients[fd];
	if (!conn)
		return ;
	ssize_t rd_bytes = 0;
	char buf[READ_BUFFER_SIZE];
	for(;(rd_bytes = recv(fd,buf,READ_BUFFER_SIZE,0)) >= 0 ;)
	{
		std::cout<<buf<<std::endl;
		conn->buffer.append(buf,rd_bytes);
	}
	
	switch (conn->state)
	{
		case Connection::READING_REQUEST_LINE:
			std::cout<<"READING_REQUEST_LINE is reached\n";
			ProcessRequestLine(conn);
			break;
		case Connection::READING_HEADERS:
			std::cout<<"READING_HEADERS is reached\n";
			ProcessHeaders(conn);
			break;
		case Connection::PROCESSING:
			std::cout<<"PROCESSING is reached\n";
			ProcessRequest(conn);
			if (conn->request->ExpectBody())
				conn->state = Connection::READING_BODY;
			else
			{
				ev.data.fd = fd;
				ev.events = EPOLLOUT | EPOLLET;
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
				conn->state = Connection::SENDING_RESPONSE;
			}
			break;
		case Connection::READING_BODY:
			// read post body
			break;
		case Connection::SENDING_RESPONSE:
			ev.data.fd = fd;
			ev.events = EPOLLOUT | EPOLLET;
			epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
			break;
		default:
			std::cout<<"default state has been reached\n";
			if (conn->state == Connection::READING_REQUEST_LINE && conn->buffer.size() >= MAX_REQUEST_LINE_LENGHT)
			{
				conn->response = new Response(414); //[sessarhi] uri too large response
				ev.data.fd = fd;
				ev.events = EPOLLOUT | EPOLLET;
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
				conn->state = Connection::SENDING_RESPONSE;
			}
			else if (conn->state == Connection::READING_HEADERS && conn->buffer.size() >= MAX_header_field_LENGHT)
			{
				conn->response = new Response(431); //[sessarhi] header field too large response
				ev.data.fd = fd;
				ev.events = EPOLLOUT | EPOLLET;
				epoll_ctl(epoll_fd, EPOLL_CTL_MOD, fd, &ev);
				conn->state = Connection::SENDING_RESPONSE;
			}
			break;
	}
}

void		HttpServer::run()
{
	for(;;)
	{
		int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, 500);
		if (event_count == -1)
		    throw HttpServerError("Epoll wait failed");
		for (int i = 0; i < event_count; ++i)
		{
			if (server_map.find(events[i].data.fd) != server_map.end())
			{
				HandleNewConnection(events[i].data.fd);
			}
			else if (events[i].events & (EPOLLIN | EPOLLOUT))
			{
				if (!CheckForEventFd(active_clients,events[i].data.fd))
				{
					active_clients.push_back(events[i]);
				}
			}
			else if (events->events & (EPOLLHUP | EPOLLERR))
			{
				std::cout<<"events->events & (EPOLLHUP | EPOLLERR)\n";
				// [sessarhi] handle errors for this fd
			}
		}
		ProcessClientsRoundRobin();
	}
}


void		HttpServer::ProcessClientsRoundRobin()
{
	if (active_clients.empty())
		return;
    int clients_count =  std::min(active_clients.size(),(size_t)CLIENT_PER_CYCLE);
	for (;clients_count--;)
	{
		struct  epoll_event client_ev = active_clients.front();
		active_clients.pop_front();
		// if (clients.find(client_ev.data.fd) == clients.end())
		// {
		// 	active_clients.pop_front();
		// 	continue;
		// }
		Connection *conn = clients[client_ev.data.fd];
		if (client_ev.events & EPOLLIN)
		{
			HandlIncommingData(client_ev.data.fd);
			if (conn->state == Connection::SENDING_RESPONSE)
			{
				client_ev.events = EPOLLOUT;
			}
				
		}
		else if (client_ev.events & EPOLLOUT)
		{
			HandlOutgoingData(client_ev.data.fd);
		}
		if (conn->state != Connection::COMPLETE)
		{
			active_clients.push_back(client_ev);
		}
	}
}

void 		HttpServer::ProcessRequest(Connection *conn)
{
	std::string host = conn->request->GetHeader("host");
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);
	if(getpeername(conn->fd,(struct sockaddr*)&client_addr,&addr_len) == -1)
	{
		return; // an action should be taken in case of error
	}
	bool is_default = true;
	int port = ntohs(client_addr.sin_port);
	std::string ip = inet_ntoa(client_addr.sin_addr);
	for (size_t i = 0; i < servers.size();++i)
	{
		for (size_t j = 0 ; j < servers[i].listen.size();++j)
		{
			if (port == servers[i].listen[j].second && ip == servers[i].listen[j].first)
			{
				if (is_default)
				{
					conn->server = &servers[i];
					is_default = false;
				}
				for (size_t k = 0; k < servers[i].server_names.size();++k)
				{
					if (servers[i].server_names[k] == host)
					{
						conn->server = &servers[i];
						break;
					}
				}
			}
		}
	}
	
}


void        HttpServer::HandlOutgoingData(int fd)
{
    std::string body =
        "<html>\r\n"
        "<head>\r\n"
        "  <title>Example Page</title>\r\n"
        "</head>\r\n"
        "<body>\r\n"
        "  <h1>Success!</h1>\r\n"
		"<img src=\"https://cdn.intra.42.fr/users/69c9ee167204b441a606d08fbcb79cca/eaboudi.jpg\">\r\n"
        "  <p><b>This is a simple example HTML page sent via C++98.</b></p>\r\n"
        "</body>\r\n"
        "</html>";
    char len_str[32];
    sprintf(len_str, "%lu", (unsigned long)body.length());

    std::string http_response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Content-Length: ";
    http_response += len_str;
    http_response += "\r\n"
        "Connection: close\r\n"
        "\r\n";
    http_response += body;
    const char* response_cstr = http_response.c_str();
    size_t response_len = http_response.length();
    ssize_t bytes_sent = send(fd, response_cstr, response_len, 0);
    if (bytes_sent < 0) 
    {
        std::cerr << "Error sending response: " << strerror(errno) << std::endl;

    }
    else if (static_cast<size_t>(bytes_sent) < response_len)
    {
        std::cerr << "Warning: Partial send. Sent " << bytes_sent << " of " << response_len << " bytes." << std::endl;
 // Indicate sending started
    }
    std::cout << "HTTP Response sent successfully (" << bytes_sent << " bytes)." << std::endl;
}

bool		HttpServer::CheckForEventFd(std::deque<struct  epoll_event>&,int fd)
{
	for (std::deque<struct  epoll_event>::iterator it = active_clients.begin()
			;it != active_clients.end();++it)
	{
		if ((*it).data.fd == fd)
			return true;
	}
	return false;
}


void		HttpServer::cleanup()
{
	for (std::map<int, Server>::iterator it = server_map.begin(); it != server_map.end(); ++it)
	{
		close(it->first);
	}
	server_map.clear();
}

HttpServer::~HttpServer()
{
	if (epoll_fd != -1)
	{
		close(epoll_fd);
	}
}	