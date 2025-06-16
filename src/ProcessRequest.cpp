/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessRequest.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/14 12:00:41 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/16 12:07:15 by sessarhi         ###   ########.fr       */
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
	if(!MatchLocation(conn))
	{
		conn->response = new Response(404);
		conn->state = Connection::SENDING_RESPONSE;
	}
	FillLocationMisseddata(conn);
	// conn->response = new Response(conn->request,conn->server);
	if (conn->request->GetMethod() == "POST")
	{
		ProcessPostRequest(conn);
	}
	else if (conn->request->GetMethod() == "GET")
	{
		
	}
	else if (conn->request->GetMethod() == "DELETE")
	{
		
	}
}
bool HttpServer::MatchLocation(Connection *conn)
{
    std::string target = conn->request->GetUri();
    size_t query_pos = target.find('?');
    if (query_pos != std::string::npos) {
        target = target.substr(0, query_pos);
    }
    std::map<std::string, LocationData>::iterator it = conn->server->locations.find(target);
    if (it != conn->server->locations.end()) {
        conn->location = &it->second;
        return true;
    }
    
    while (!target.empty()) {
        size_t last_slash = target.find_last_of('/');
        
        if (last_slash == std::string::npos) {
            break;
        }
        if (last_slash == 0) {
            target = "/";
        } else {
            target = target.substr(0, last_slash);
        }
        it = conn->server->locations.find(target);
        if (it != conn->server->locations.end()) {
            conn->location = &it->second;
            return true;
        }
        if (target == "/") {
            break;
        }
    }
    it = conn->server->locations.find("/");
    if (it != conn->server->locations.end()) {
        conn->location = &it->second;
        return true;
    }
    return false;
}


bool		HttpServer::ProcessPostRequest(Connection *conn)
{
	if (conn->location->methods.find("POST") == conn->location->methods.end())
	{
		conn->response = new Response(405);
		return false;
	}	
	return true;
}

void 		HttpServer::FillLocationMisseddata(Connection *conn)
{
	if (conn->location->root.empty())
	{
		conn->location->root = conn->server->root;
	}
	if (conn->location->index.empty() && !conn->server->index.empty())
	{
		conn->location->index = conn->server->index;
	}
	if (!conn->location->has_redirect && conn->server->has_redirect)
	{
		conn->location->has_redirect = conn->server->has_redirect;
		conn->location->redirect.first = conn->server->redirect.first;
		conn->location->redirect.second = conn->server->redirect.second;
	}
	if (!conn->location->autoindex_set && conn->server->autoindex_set)
	{
		conn->location->autoindex = conn->server->autoindex;
	}
	if (conn->location->cgi.empty() && !conn->server->cgi.empty())
	{
		conn->location->cgi = conn->server->cgi;
	}
	if (!conn->location->upload_set && conn->server->upload_set)
	{
		conn->location->upload = conn->server->upload;
		conn->location->upload_store = conn->server->upload_store;
	}
	if (conn->location->error_pages.empty() && !conn->server->error_pages.empty())
	{
		conn->location->error_pages = conn->server->error_pages;
	}
	if (!conn->location->max_body_size && conn->server->max_body_size)
	{
		conn->location->max_body_size = conn->server->max_body_size;
	}
}
