/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:13:01 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/21 13:34:18 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef			HTTPSERVER_HPP
# define 		HTTPSERVER_HPP

#include		"ConfigData.hpp"

#ifdef __linux__
    #include <sys/epoll.h>
#elif defined(__APPLE__)
    #include <sys/event.h>
    #include <sys/time.h>
#endif

#ifdef __linux__
    #define READ_EVENT EPOLLIN
    #define WRITE_EVENT EPOLLOUT
    #define ERROR_EVENT EPOLLERR
    #define HUP_EVENT EPOLLHUP
    #define EDGE_TRIGGERED EPOLLET
#elif defined(__APPLE__)
    #define READ_EVENT		0x01
    #define WRITE_EVENT 	0x02
    #define ERROR_EVENT 	0x04
    #define HUP_EVENT		0x08
    #define EDGE_TRIGGERED EV_CLEAR
#endif

#include		<unistd.h>
#include		<exception>
#include		<sys/socket.h>
#include		<netinet/in.h>
#include		<netdb.h>
#include		<cstring>
#include		<sstream>
#include		<fcntl.h>
#include		<iostream>
#include		"Connection.hpp"
#include		<errno.h>
#include		<deque>
#include		<algorithm>
#include		<cstdio>
#include 		<arpa/inet.h>
#include		<sys/types.h>
#include		"Exceptions.hpp"

class Connection;
#define			MAX_EVENTS					1024
#define			CLIENT_PER_CYCLE			1024
#define			MAX_REQUEST_LINE_LENGHT		8000
#define			MAX_header_field_LENGHT		24000
#define			READ_BUFFER_SIZE			64000

struct PlatformEvent 
{
    int fd;
    int events;
    void* data;
};


class HttpServer
{

public:

	HttpServer(std::vector<Server> &);
	
	~HttpServer();
	
	void		run();
	
	void		cleanup();

	static bool 		isValueCaseInsensitive(const std::string& headerName);
private:

    static const       std::map<std::string, bool>& getHeaderCaseMap();

	void		init();

	int			claculateBufferSize();
	
	
	bool		read(Connection *);
	
	void		ClientCleanUp(int fd);
	
	void		HandleNewConnection(int fd);
	
	void		HandlIncommingData(int fd);

	void		HandlOutgoingData(int fd);
	
	void		ProcessClientsRoundRobin();

	void		ProcessRequestLine(Connection *);
	
	void		ProcessHeaders(Connection *);
	
	bool		CheckForEventFd(int fd);
	
	void		ProcessRequest(Connection *);
	
	void		SetSocketForWrite(Connection *);
	
	void		SetSocketForRead(Connection *);
	
	bool		MatchLocation(Connection *);

	bool		ProcessPostRequest(Connection *);
	
	void 		FillLocationMisseddata(Connection *);
	
	void 		SetClientSocketToNonblocking(int fd);

	void 		SetServerSocketToNonblocking(int fd);

    

	void		SetTimeOut();
	
	int 		CreateEvent();
	
    int 		AddEvent(int fd, int events);

    int 		ModifyEvent(int fd, int events);
	
    int 		RemoveEvent(int fd);
	
    int 		WaitForEvents(PlatformEvent* events, int max_events, int timeout);
	
    int 		event_fd;
    
#ifdef __linux__
    struct epoll_event ev, events[MAX_EVENTS];
#elif defined(__APPLE__)
    struct kevent kevents[MAX_EVENTS];
    struct kevent change_list[4];
	struct timespec *pts;
	struct timespec ts;
    int change_count;
#endif
	
	size_t							buf_size;
	
	std::vector<char>				buf;
    
	std::vector<Server>				&servers;
	
	std::map<int, Server> 			server_map;
	
	std::map<int , Connection* > 	clients;
	
	std::deque<PlatformEvent>		active_clients;

	const std::map<std::string,bool>&		headerCaseMap;
};


#endif