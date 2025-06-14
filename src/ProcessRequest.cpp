/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessRequest.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/14 12:00:41 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/14 12:03:40 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpServer.hpp"


void		HttpServer::ProcessRequestLine(Connection *conn)
{
	size_t end = conn->buffer.find("\r\n");
	if (end  != std::string::npos)
	{
		std::string line = conn->buffer.substr(0,end);
		if (line.empty())
        {
            conn->buffer.erase(0, end + 2);
            return;
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
				bool IsValid = conn->request->ParseHeaders(conn->buffer.substr(0,end + 2));
				conn->buffer.erase(0,end + 4);
				if (IsValid)
				{
					conn->state = Connection::PROCESSING;
				}
				else
				{
					conn->response = new Response(conn->request->GetStatus());
					conn->state = Connection::SENDING_RESPONSE;
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
	MatchLocation(conn);
	
	conn->response = new Response(conn->request,conn->server);
	if (conn->request->GetMethod() == "GET")
	{
		
	}
	else if (conn->request->GetMethod() == "POST")
	{
		
	}
	else if (conn->request->GetMethod() == "DELETE")
	{
		
	}
}

bool		HttpServer::MatchLocation(Connection *conn)
{
	std::string target =conn->request->GetUri();
	target = target.find_last_of('?') == std::string::npos ? target : 
		target.substr(target.find_last_of('?'));
	std::map<std::string,LocationData>::iterator it = conn->server->locations.find(target);
	while (target.size() > 0)
	{
		
		if (it != conn->server->locations.end())
		{
			conn->location = &it->second;
			break;
		}
		size_t last_slash = target.find_last_of('/');
		if (last_slash != std::string::npos)
		{
			target = target.substr(0,last_slash);
		}
		else
		{
			std::map<std::string,LocationData>::iterator it = conn->server->locations.find("/");
			if(it == conn->server->locations.end())
			{
				//handel no location matching
			}
		}
	}
	return true;
}

