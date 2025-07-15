/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:13:01 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/15 20:53:55 by sessarhi         ###   ########.fr       */
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


class Connection;
#define			MAX_EVENTS					1024
#define			READ_BUFFER_SIZE			64000
#define			CLIENT_PER_CYCLE			512
#define			MAX_REQUEST_LINE_LENGHT		8000 //RFC 9112,
#define			MAX_header_field_LENGHT		8000

struct PlatformEvent {
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


private:	

	void		init();
	
	void		SetSocketToNonblocking(int fd);
	
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
	
	
	int CreateEvent();
    int AddEvent(int fd, int events);
    int ModifyEvent(int fd, int events);
    int RemoveEvent(int fd);
    int WaitForEvents(PlatformEvent* events, int max_events, int timeout);
    int event_fd;
    
#ifdef __linux__
    struct epoll_event ev, events[MAX_EVENTS];
#elif defined(__APPLE__)
    struct kevent kevents[MAX_EVENTS];
    struct kevent change_list[MAX_EVENTS];
    int change_count;
#endif
	
	std::vector<Server>				&servers;
	
	std::map<int, Server> 			server_map;
	
	std::map<int , Connection* > 	clients;
	
	 std::deque<PlatformEvent> active_clients;
	
};


class HttpServerError : public std::exception
{
	public:
	
		HttpServerError(const char *msg) : message(msg) {}
		
		virtual const char* what() const throw()
		{
			return message;
		}
		
		virtual ~HttpServerError() throw() {}
		
	private:

		const char *message;
};


#endif