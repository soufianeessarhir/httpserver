/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 18:08:39 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/21 20:48:30 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpServer.hpp"


HttpServer::HttpServer(std::vector<Server> &srvs) :buf(claculateBufferSize()), servers(srvs),headerCaseMap(getHeaderCaseMap())
{
    event_fd = CreateEvent();
    if (event_fd == -1)
        perror("Event queue creation failed");
#if defined(__APPLE__)
    change_count = 0;
	pts = NULL;
#endif
    this->init();
}

int			HttpServer::claculateBufferSize()
{
    int recv_size;
    int send_size;
    socklen_t optlen = sizeof(int);
	int target_buf;

    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
        perror("failed to create a socket");
	for (int i = 10; i > 0; --i) 
	{
		target_buf = (1024 * 1024) * i;
		if (setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &target_buf, sizeof(target_buf)) == 0 &&
			setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &target_buf, sizeof(target_buf)) == 0)
			break;
	}
    getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &send_size, &optlen);
    getsockopt(fd, SOL_SOCKET, SO_RCVBUF, &recv_size, &optlen);
    close(fd);
    buf_size = std::max(recv_size, send_size);
    return buf_size;
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

void HttpServer::SetTimeOut()
{
	
#ifdef __APPLE__
    if (active_clients.empty())
        pts = NULL;
    else
    {
        ts.tv_sec  = 0;
        ts.tv_nsec = 5 * 1000 * 1000; 
        pts        = &ts;
    }
#endif

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
#ifdef		__linux__

	ev.events = events;
	if (events & EDGE_TRIGGERED)
		ev.events |= EPOLLET;
	ev.data.fd = fd;
	result = epoll_ctl(event_fd, EPOLL_CTL_ADD, fd, &ev);

#elif		defined(__APPLE__)

    SetTimeOut();
    int flags = EV_ADD;
    if (events & EDGE_TRIGGERED)
        flags |= EV_CLEAR;
    if (events & READ_EVENT)
        EV_SET(&change_list[change_count++], fd, EVFILT_READ, flags, 0, 0, NULL);
    if (events & WRITE_EVENT)
        EV_SET(&change_list[change_count++], fd, EVFILT_WRITE, flags, 0, 0, NULL);
    result = kevent(event_fd, change_list, change_count, NULL, 0, pts);
    change_count = 0;
    if (result < 0 && errno != ENOENT && errno != EBADF)
       throw  HttpClientError("AddEvent failed",fd);

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

	SetTimeOut();
	change_count = 0;
	EV_SET(&change_list[change_count++], fd, EVFILT_READ, EV_DISABLE, 0, 0, NULL);
	EV_SET(&change_list[change_count++], fd, EVFILT_WRITE, EV_DISABLE, 0, 0, NULL);
	if (events & READ_EVENT)
		EV_SET(&change_list[change_count++], fd, EVFILT_READ, EV_ENABLE | EV_CLEAR, 0, 0, NULL);
	if (events & WRITE_EVENT)
		EV_SET(&change_list[change_count++], fd, EVFILT_WRITE, EV_ENABLE | EV_CLEAR, 0, 0, NULL);
	result = kevent(event_fd, change_list, change_count, NULL, 0, pts);
    change_count = 0;
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
    result = kevent(event_fd, change_list, change_count, NULL, 0, pts);
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

	SetTimeOut(); 
  	event_count = kevent(event_fd, NULL, 0, kevents, max_events, pts);
	for (int i = 0; i < event_count; i++)
	{
		platform_events[i].fd = kevents[i].ident;
		platform_events[i].data = kevents[i].udata;
		platform_events[i].events = 0;
		if (kevents[i].filter == EVFILT_READ) 		platform_events[i].events = READ_EVENT; 
		if (kevents[i].filter == EVFILT_WRITE) 		platform_events[i].events = WRITE_EVENT;
		if (kevents[i].flags  == EV_ERROR) 			platform_events[i].events = ERROR_EVENT;
		if (kevents[i].flags  == EV_EOF)			platform_events[i].events = HUP_EVENT;
	}
	if (event_count < 0  && errno != ENOENT && errno != EBADF)
		perror("WaitForEvents failed");
		
#endif
    return event_count;
}

void		HttpServer::CreateSocket(struct addrinfo *p,int &sockfd,struct addrinfo *res)
{
	for (p = res; p != NULL; p = p->ai_next)
	{
		sockfd = socket(p->ai_family, p->ai_socktype, 0);
		if (sockfd == -1) continue;
		int optval = 1;
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0)
		{
			close(sockfd);	freeaddrinfo(res);	perror("setsockopt(SO_REUSEADDR) failed");
		}
		SetServerSocketToNonblocking(sockfd);
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) != -1) break;
		close(sockfd);
		sockfd = -1;
	}
	freeaddrinfo(res);
	if (p == NULL)
		perror("Socket binding failed");
	if (listen(sockfd, SOMAXCONN) == -1)
	{ 
		close(sockfd); perror("Socket listening failed");
	}
}

void HttpServer::init()
{
    struct addrinfo hints, *res, *p = NULL;

    for (size_t i = 0; i < servers.size(); ++i)
    {
        for (size_t j = 0; j < servers[i].listen.size(); ++j)
        {
            int sockfd = -1;
            const std::string& cfgHost = servers[i].listen[j].first;
            const int          portNum = servers[i].listen[j].second;
            const std::string hostForCompare = cfgHost.empty() ? "0.0.0.0" : cfgHost;
			if (servers[i].isvirtual && std::find(servers[i].virtual_listen.begin(),
                servers[i].virtual_listen.end(), std::make_pair(hostForCompare, portNum)) != servers[i].virtual_listen.end()) continue;
            memset(&hints, 0, sizeof(hints));
            hints.ai_family   = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags    = cfgHost.empty() ? AI_PASSIVE : 0;
            const char* node  = cfgHost.empty() ? NULL : cfgHost.c_str();
			std::stringstream ss;
			ss << portNum;
			std::string portStr = ss.str();
            if (getaddrinfo(node, portStr.c_str(), &hints, &res) != 0)
                perror("Getaddrinfo failed");
			CreateSocket(p,sockfd,res);
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
		{ close(fd); perror("fcntl get failed on server socket");}
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		{ close(fd); perror("fcntl set failed on server socket");}
}

void HttpServer::SetClientSocketToNonblocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        perror("fcntl get failed on client socket");
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
        perror("fcntl set failed on client socket");
}

void		HttpServer::SetSocketForWrite(Connection *conn)
{
	ModifyEvent(conn->fd,WRITE_EVENT);
	conn->state = Connection::SENDING_RESPONSE;
}

void		HttpServer::SetSocketForRead(Connection *conn)
{
    conn->state = Connection::READING_REQUEST_LINE;
    ModifyEvent(conn->fd,READ_EVENT);
}

bool		HttpServer::read(Connection *conn)
{
	ssize_t n = 0;
	for (;;) 
	{
        n = recv(conn->fd,buf.data(),buf_size, MSG_DONTWAIT);
        if (n > 0)
            conn->buffer.append(buf.data(), n);
        else if (n == 0)
                throw HttpClientError("connection close by peer", conn->fd);
		else
		{ 
			if(conn->buffer.size() == 0) 
				return false; 
			return true;
		}
    }
}


void		HttpServer::HandleNewConnection(int fd)
{
	for (;;)
	{
		struct sockaddr_in s;
		socklen_t socklen = sizeof(s);
		int client_fd = accept(fd, (sockaddr *)&s, &socklen);
		if (clients.size() > 1024)
			return;
		if (client_fd == -1)
			return;
		Connection *conn = new Connection(client_fd);
		char ipstr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &s.sin_addr, ipstr, sizeof(ipstr));//should be removed
		conn->ip 			= ipstr;
		conn->port 			= ntohs(s.sin_port);
		SetClientSocketToNonblocking(client_fd);
		AddEvent(client_fd,READ_EVENT | EDGE_TRIGGERED);
		clients[client_fd] 	= conn;
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
					continue_processing = conn->state != Connection::READING_REQUEST_LINE;
				break;
				
			case Connection::READING_HEADERS:
			
				ProcessHeaders(conn);
					continue_processing = conn->state == Connection::PROCESSING;
				break;
				
			case Connection::PROCESSING:

				ProcessRequest(conn);
				//the next checks should be in ProcessRequest
				if (conn->request->ExpectBody() && conn->request->GetMethod() == "POST")
				{
					if (conn->state == Connection::PROCESSING)
						conn->state     = Connection::READING_BODY;
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
				try
				{
					fd = platform_events[i].fd;
					ev = platform_events[i].events;
					if (ev == HUP_EVENT || ev == ERROR_EVENT)
					{ClientCleanUp(fd); continue ;}
					else if (server_map.find(fd) != server_map.end())
						HandleNewConnection(fd);
					else if (ev & (READ_EVENT | WRITE_EVENT))
						if (!CheckForEventFd(fd)) active_clients.push_back(platform_events[i]);
				}
				catch(const HttpClientError &e)
				{
					std::cerr << e.what() << '\n';
					ClientCleanUp(e.client_fd);
				}
			}
			if (!active_clients.empty())
				ProcessClientsRoundRobin();
		}
		catch(const HttpClientError &e)
		{
			std::cerr << e.what() << '\n';
			ClientCleanUp(e.client_fd);
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
			if (clients.find(client_ev.fd) == clients.end())
				continue;
			if (conn->state == Connection::READING_REQUEST_LINE)
                client_ev.events = READ_EVENT;
		}
		if (conn && conn->state != Connection::COMPLETE )
			active_clients.push_back(client_ev);
    }
}

void        HttpServer::HandlOutgoingData(int fd)
{
    Connection *conn = clients[fd];
	excuteGetMethod(conn);
	if (conn->state == Connection::COMPLETE)
	{
		if (conn->response->GetMethod() == Error)
			{ ClientCleanUp(conn->fd); return;}
		else if (conn->request->CheckField("connection") && conn->request->GetHeader("connection") != "keep-alive")
			{ClientCleanUp(conn->fd);return;}
		SetSocketForRead(conn);
		conn->Reset();
	}
}
void HttpServer::ClientCleanUp(int fd)
{
    RemoveEvent(fd);
    for (std::deque<PlatformEvent>::iterator it = active_clients.begin();
         it != active_clients.end(); )
    {
        if (it->fd == fd) it = active_clients.erase(it);
        else ++it;
    }
    Connection *conn = clients[fd];
    if (conn) 
	{
		clients.erase(fd);
		delete conn;
	}
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

