/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   HttpServer.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:13:01 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/16 10:26:01 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef			HTTPSERVER_HPP
# define 		HTTPSERVER_HPP

#include		"ConfigData.hpp"
#include		<sys/epoll.h>
#include		<unistd.h>
#include		<exception>
#include		<sys/socket.h>
#include		<netinet/in.h>
#include		<netdb.h>
#include		<cstring>
#include		<sstream>
#include		<fcntl.h>
#include			<iostream>
#include		"Connection.hpp"
#include		<errno.h>
#include		<deque>
#include		<algorithm>
#include		<cstdio>
#include 		<arpa/inet.h>
class Connection;
#define			MAX_EVENTS					1024
#define			READ_BUFFER_SIZE			4096
#define			CLIENT_PER_CYCLE			512
#define			MAX_REQUEST_LINE_LENGHT		8000 //RFC 9112,
#define			MAX_header_field_LENGHT		1024


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
	
	bool		CheckForEventFd(std::deque<struct  epoll_event>&,int fd);
	
	void		ProcessRequest(Connection *);
	
	void		SetSocketForWrite(Connection *);
	
	void		SetSocketForRead(Connection *);
	
	bool		MatchLocation(Connection *);

	bool		ProcessPostRequest(Connection *);
	
	void 		FillLocationMisseddata(Connection *);
	
	

	



	
	
	int								epoll_fd;
	
	struct  epoll_event 			ev, events[MAX_EVENTS];
	
	std::vector<Server>				&servers;
	
	std::map<int, Server> 			server_map;
	
	std::map<int , Connection* > 	clients;
	
	std::deque<struct epoll_event>	active_clients;
	
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