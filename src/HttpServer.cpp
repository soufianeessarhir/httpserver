/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 18:08:39 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/16 20:10:39 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpServer.hpp"

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
void		HttpServer::SetSocketForWrite(Connection *conn)
{
	
	ev.data.fd = conn->fd;
	ev.events = EPOLLOUT | EPOLLET;
	epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn->fd, &ev);
	conn->state = Connection::SENDING_RESPONSE;
}

void HttpServer::SetSocketForRead(Connection *conn)
{
    ev.data.fd = conn->fd;
    ev.events = EPOLLIN | EPOLLET;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn->fd, &ev) == -1)
    {
        perror("epoll_ctl: change to read mode");
        conn->state = Connection::COMPLETE;
        return;
    }
    conn->state = Connection::READING_REQUEST_LINE; // For keep-alive
}

void		HttpServer::HandleNewConnection(int fd)
{
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

void		HttpServer::HandlIncommingData(int fd)
{
	Connection *conn = clients[fd];
	if (!conn)
		return ;
	ssize_t rd_bytes = 0;
	char buf[READ_BUFFER_SIZE];
	for(;(rd_bytes = recv(fd,buf,READ_BUFFER_SIZE,MSG_DONTWAIT)) > 0 ;)
	{
		conn->buffer.append(buf,rd_bytes);
	}
	if (rd_bytes == 0) {
		//[sessarhi] Connection should closed closed
		return;
	}
	bool continue_processing = true;
	while (continue_processing)
	{
		continue_processing = false;
		switch (conn->state)
		{
			case Connection::READING_REQUEST_LINE:
			
				if (conn->state == Connection::READING_REQUEST_LINE && conn->buffer.size() >= MAX_REQUEST_LINE_LENGHT)
				{
					conn->response = new Response(414); //[sessarhi] uri too large response
					SetSocketForWrite(conn);
				}
				ProcessRequestLine(conn);
				if (conn->state == Connection::READING_HEADERS)
					continue_processing = true;
				break;
				
			case Connection::READING_HEADERS:
			
				if (conn->state == Connection::READING_HEADERS && conn->buffer.size() >= MAX_header_field_LENGHT)
				{
					conn->response = new Response(431); //[sessarhi] header field too large response
					SetSocketForWrite(conn);
					conn->state = Connection::SENDING_RESPONSE;
				}
				ProcessHeaders(conn);
				if (conn->state == Connection::PROCESSING)
					continue_processing = true;
				break;
				
			case Connection::PROCESSING:
			
				ProcessRequest(conn);
				if (conn->request->ExpectBody())
				{
					conn->state = Connection::READING_BODY;
					continue_processing = true;
				}
				else
					SetSocketForWrite(conn);
				break;
				
			case Connection::READING_BODY:
			
				break;
				
			case Connection::SENDING_RESPONSE:
			
				SetSocketForWrite(conn);
				break;
			default :
				break;
		}
		
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
			else if (events[i].events  & EPOLLHUP)
			{
				// [sessarhi] handle errors for this fd
			}
			else if (events[i].events  & EPOLLERR)
			{
				
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
		if (clients.find(client_ev.data.fd) == clients.end())
		{
			active_clients.pop_front();
			continue;
		}
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


void        HttpServer::HandlOutgoingData(int fd)
{
    Connection *conn = clients[fd];
    // if (!conn || !conn->response)
    // {
	// 	std::cout << "3\n";
	// 	return;
    // }
	std::cout<< conn->response->GET<<std::endl;
    conn->response->GET = new GetMethodResponse(conn->request->GetStatus(), "/home/eaboudi/Desktop/httpserver/src/index.html");
    conn->response->GET->SendStatusLine(conn);
    conn->response->GET->SendHeaders(conn);
    if (conn->response->GET->GetStatusCode() == 200)
    {
        conn->response->GET->SendBody(conn);
        if (conn->response->GET->GetBody().empty())
            conn->state = Connection::COMPLETE; // No body to send
    }
    else
        conn->state = Connection::COMPLETE; // Error response sent
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
