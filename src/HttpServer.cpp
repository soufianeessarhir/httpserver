/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 18:08:39 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/16 21:21:21 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpServer.hpp"




HttpServer::HttpServer(std::vector<Server> &srvs) : servers(srvs),headerCaseMap(getHeaderCaseMap())
{
    event_fd = CreateEvent();
    if (event_fd == -1)
        throw HttpServerError("Event queue creation failed");
#if defined(__APPLE__)
    change_count = 0;
#endif
    this->init();
}


const std::map<std::string, bool>& HttpServer::getHeaderCaseMap()
{
    static std::map<std::string, bool> headerCaseMap;
	headerCaseMap["transfer-encoding"] = true;
	headerCaseMap["content-encoding"] = true;
	headerCaseMap["connection"] = true;
	headerCaseMap["content-type"] = true;
	headerCaseMap["accept"] = true;
	headerCaseMap["accept-encoding"] = true;
	headerCaseMap["expect"] = true;
	headerCaseMap["allow"] = true;
	headerCaseMap["etag"] = false;
	headerCaseMap["if-match"] = false;
	headerCaseMap["if-none-match"] = false;
	headerCaseMap["set-cookie"] = false;
	headerCaseMap["cookie"] = false;
	headerCaseMap["content-disposition"] = false;
	headerCaseMap["location"] = false;
	headerCaseMap["referer"] = false;
    return headerCaseMap;
}

bool HttpServer::CheckForEventFd(int fd)
{
    for (std::deque<PlatformEvent>::iterator it = active_clients.begin();
    it != active_clients.end(); ++it)
    {
        if (it->fd == fd)
            return true;
    }
    return false;
}


int HttpServer::CreateEvent()
{
#ifdef __linux__
    return epoll_create1(0);
#elif defined(__APPLE__)
    return kqueue();
#endif
}

int HttpServer::AddEvent(int fd, int events)
{
	int result;
#ifdef __linux__
    ev.events = events | EDGE_TRIGGERED;
    ev.data.fd = fd;
    result =  epoll_ctl(event_fd, EPOLL_CTL_ADD, fd, &ev);
#elif defined(__APPLE__)
    if (events & READ_EVENT)
	{
        EV_SET(&change_list[change_count], fd, EVFILT_READ, EV_ADD | EDGE_TRIGGERED, 0, 0, NULL);
        change_count++;
    }
    if (events & WRITE_EVENT)
	{
        EV_SET(&change_list[change_count], fd, EVFILT_WRITE, EV_ADD | EDGE_TRIGGERED, 0, 0, NULL);
        change_count++;
    }
    result = kevent(event_fd, change_list, change_count, NULL, 0, NULL);
    change_count = 0;
	if (result < 0 && errno != ENOENT && errno != EBADF)
		throw HttpClientError("AddEvent failed",fd);
#endif
    return result;
}

int HttpServer::ModifyEvent(int fd, int events)
{
	int result;
#ifdef __linux__
    ev.events = events | EDGE_TRIGGERED;
    ev.data.fd = fd;
    result =  epoll_ctl(event_fd, EPOLL_CTL_MOD, fd, &ev);
#elif defined(__APPLE__)
	change_count = 0;
	EV_SET(&change_list[change_count++], fd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
	EV_SET(&change_list[change_count++], fd, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
	if (events & READ_EVENT)
		EV_SET(&change_list[change_count++], fd, EVFILT_READ, EV_ENABLE | EV_CLEAR, 0, 0, NULL);
	if (events & WRITE_EVENT)
		EV_SET(&change_list[change_count++], fd, EVFILT_WRITE, EV_ENABLE | EV_CLEAR, 0, 0, NULL);
	result = kevent(event_fd, change_list, change_count, NULL, 0, NULL);
    change_count = 0;
	if (result < 0 && errno != ENOENT && errno != EBADF)
		throw HttpClientError("ModifyEvent",fd);
#endif
    return result;
}

int HttpServer::RemoveEvent(int fd)
{
	int result;
#ifdef __APPLE__
    change_count = 0;
    EV_SET(&change_list[change_count], fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
    change_count++;
    EV_SET(&change_list[change_count], fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
    change_count++;
    result = kevent(event_fd, change_list, change_count, NULL, 0, NULL);
    if (result < 0 && errno != ENOENT && errno != EBADF)
      throw HttpClientError("kevent EV_DELETE",fd);
    change_count = 0;
#else
    result = epoll_ctl(event_fd, EPOLL_CTL_DEL, fd, NULL);
#endif
    return result;
}

int HttpServer::WaitForEvents(PlatformEvent* platform_events, int max_events, int timeout = 0)
{
	int event_count;
	if (active_clients.empty())
		timeout = -1;
#ifdef __linux__
    event_count = epoll_wait(event_fd, events, max_events, timeout);
    for (int i = 0; i < event_count; i++) {
        platform_events[i].fd = events[i].data.fd;
        platform_events[i].events = events[i].events;
        platform_events[i].data = NULL;
    }
#elif defined(__APPLE__)
    struct timespec ts;
    struct timespec *pts = NULL;
    if (timeout == -1)
        pts = NULL;
    else {
        ts.tv_sec = 0 ;
        ts.tv_nsec = 0;
        pts = &ts;
    }
    event_count = kevent(event_fd, NULL, 0, kevents, max_events, pts);
	for (int i = 0; i < event_count; i++)
	{
		platform_events[i].fd = kevents[i].ident;
		platform_events[i].data = kevents[i].udata;
		platform_events[i].events = 0;
		if (kevents[i].filter == EVFILT_READ)
			platform_events[i].events |= READ_EVENT; 
		if (kevents[i].filter == EVFILT_WRITE)
			platform_events[i].events |= WRITE_EVENT;
		if (kevents[i].flags & EV_ERROR)
			platform_events[i].events |= ERROR_EVENT;
		if (kevents[i].flags & EV_EOF)
			platform_events[i].events |= HUP_EVENT;
	}
	if (event_count < 0  && errno != ENOENT && errno != EBADF)
		throw HttpServerError("WaitForEvents failed");
#endif
    return event_count;
}

void HttpServer::init()
{
    struct addrinfo hints, *res, *p;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        for (size_t j = 0; j < servers[i].listen.size(); ++j)
        {
            int sockfd = -1;
            const std::string& cfgHost = servers[i].listen[j].first;
            const int          portNum = servers[i].listen[j].second;
            const std::string hostForCompare = cfgHost.empty() ? "0.0.0.0" : cfgHost;
			if (servers[i].isvirtual &&
                std::find(servers[i].virtual_listen.begin(),
                servers[i].virtual_listen.end(),
                std::make_pair(hostForCompare, portNum)) != servers[i].virtual_listen.end())
                continue;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family   = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags    = cfgHost.empty() ? AI_PASSIVE : 0;
            const char* node   = cfgHost.empty() ? NULL : cfgHost.c_str();
			std::stringstream ss;
			ss << portNum;
			std::string portStr = ss.str();
            if (getaddrinfo(node, portStr.c_str(), &hints, &res) != 0)
                throw HttpServerError("Getaddrinfo failed");
            for (p = res; p != NULL; p = p->ai_next)
            {
                sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
                if (sockfd == -1)
                    continue;
                int optval = 1;
                if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
                {
                    close(sockfd);
                    freeaddrinfo(res);
                    throw HttpServerError("setsockopt(SO_REUSEADDR) failed");
                }
                SetServerSocketToNonblocking(sockfd);
                if (bind(sockfd, p->ai_addr, p->ai_addrlen) != -1)
                    break;
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
            try
			{
                AddEvent(sockfd, READ_EVENT);	
			}
            catch (const HttpClientError &e)
			{
                std::cerr << e.what() << '\n';
			}
            server_map[sockfd] = servers[i];
        }
    }
}

void HttpServer::SetServerSocketToNonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
	{
		close(fd);
        throw HttpServerError("fcntl get failed");
	}
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		close(fd);
        throw HttpServerError("fcntl set failed");
	}
}
void HttpServer::SetClientSocketToNonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw HttpServerError("fcntl get failed");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        throw HttpServerError("fcntl set failed");
}
void		HttpServer::SetSocketForWrite(Connection *conn)
{
	ModifyEvent(conn->fd,WRITE_EVENT);
	conn->state = Connection::SENDING_RESPONSE;
}

void		HttpServer::SetSocketForRead(Connection *conn)
{
    ModifyEvent(conn->fd,READ_EVENT);
    conn->state = Connection::READING_REQUEST_LINE;
}

bool		HttpServer::read(Connection *conn)
{
	ssize_t n = 0;
	for (;;) 
	{
        n = recv(conn->fd,buf, READ_BUFFER_SIZE, MSG_DONTWAIT);
        if (n > 0)
            conn->buffer.append(buf, n);
        else if (n == 0)
                throw HttpClientError("connection close by peer", conn->fd);
		else
			if(conn->buffer.size() == 0)
				return false;
			return true;
    }
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
				break;
			else
				throw HttpServerError("Accept failed");
		}
		Connection *conn = new Connection(client_fd);
		struct sockaddr_in *s = (struct sockaddr_in *)&client_sock;
		char ipstr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof(ipstr));
		conn->ip = ipstr;
		conn->port = ntohs(s->sin_port);
		std::cout<< " " << ipstr << " " << conn->port<< std::endl;
		clients[client_fd] = conn;
		SetClientSocketToNonblocking(client_fd);
		AddEvent(client_fd,READ_EVENT | EDGE_TRIGGERED);
	}
}

void		HttpServer::HandlIncommingData(int fd)
{
	Connection *conn = clients[fd];
	if (!conn)
		return ;
	if (!read(conn))
		return;
	bool continue_processing = true;
	while (continue_processing)
	{
		continue_processing = false;
		switch (conn->state)
		{
			case Connection::READING_REQUEST_LINE:
			
				ProcessRequestLine(conn);
				if (conn->state == Connection::READING_HEADERS  || conn->state == Connection::SENDING_RESPONSE)
					continue_processing = true;
				break;
				
			case Connection::READING_HEADERS:
			
				ProcessHeaders(conn);
				if (conn->state == Connection::PROCESSING)
					continue_processing = true;
				break;
				
			case Connection::PROCESSING:

				ProcessRequest(conn);
				if (conn->request->ExpectBody() && conn->request->GetMethod() == "POST")
				{
					if (conn->state == Connection::PROCESSING)
						conn->state = Connection::READING_BODY;
					continue_processing = true;
				}
				else
					SetSocketForWrite(conn);
				break;
				
			case Connection::READING_BODY:
				if(conn->post->transfer_type == Post::CHUNKED)
					conn->post->ProcessChunck();
				else
					conn->post->ProcessContentLength();
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
	PlatformEvent platform_events[MAX_EVENTS];
	int event_count;
	int fd;
	short ev;
	for(;;)
	{
		try
		{
			event_count = WaitForEvents(platform_events, MAX_EVENTS);
			for (int i = 0; i < event_count; ++i)
			{
				fd = platform_events[i].fd;
				ev = platform_events[i].events;
				if (ev & (HUP_EVENT | ERROR_EVENT))
						ClientCleanUp(fd);
				if (server_map.find(fd) != server_map.end())
					HandleNewConnection(fd);
				else if (ev & (READ_EVENT | WRITE_EVENT))
					if (!CheckForEventFd(fd))
						active_clients.push_back(platform_events[i]);
			}
			if (!active_clients.empty())
				ProcessClientsRoundRobin();
		}
		catch(const HttpClientError &e)
		{
			std::cerr << e.what() << '\n';
			
			ClientCleanUp(e.client_fd);
		}
		catch(const HttpServerError &e)
		{
			// std::cerr << e.what() << '\n';
			return;
		}
	}
}


void		HttpServer::ProcessClientsRoundRobin()
{
    if (active_clients.empty())
        return;
    int clients_count =  std::min(active_clients.size(),(size_t)CLIENT_PER_CYCLE);
    for (;clients_count--;)
    {
        struct PlatformEvent client_ev = active_clients.front();
        active_clients.pop_front();
        if (clients.find(client_ev.fd) == clients.end())
            continue;
        Connection *conn = clients[client_ev.fd];
        if (client_ev.events & READ_EVENT)
        {
            HandlIncommingData(client_ev.fd);
            if (conn->state == Connection::SENDING_RESPONSE)
                client_ev.events = WRITE_EVENT;
        }
        else if (client_ev.events & WRITE_EVENT)
		{
            HandlOutgoingData(client_ev.fd);
			if (conn->state == Connection::READING_REQUEST_LINE)
                client_ev.events = READ_EVENT;
		}
		if (conn->state != Connection::COMPLETE )
			active_clients.push_back(client_ev);
    }
}

void        HttpServer::HandlOutgoingData(int fd)
{
    Connection *conn = clients[fd];
	excuteGetMethod(conn);
	if ( conn->state == Connection::COMPLETE && conn->response->GetMethod() != Error)
	{
		SetSocketForRead(conn);
		conn->Reset();
	}
	
}
void		HttpServer::ClientCleanUp(int fd)
{
	shutdown(fd,SHUT_WR);
	RemoveEvent(fd);
	for (std::deque<PlatformEvent>::iterator it = active_clients.begin();it != active_clients.end(); )
	{
		if (it->fd == fd)
			it = active_clients.erase(it);
		else
			++it;
	}
	Connection *conn = clients[fd];
	delete conn;
	conn = NULL;
	clients.erase(fd);
	close(fd);
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
	if (event_fd != -1)
	{
		close(event_fd);
	}
}

