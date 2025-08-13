/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ProcessRequest.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/14 12:00:41 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/13 22:10:53 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "HttpServer.hpp"

bool HttpServer::isValueCaseInsensitive(const std::string& headerName) {

    const std::map<std::string, bool>& map = HeaderValueCase::get();
    std::map<std::string, bool>::const_iterator it = map.find(headerName);
    if (it != map.end())
        return it->second;
    return false;
}
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
		}
		else
		{
			conn->response =  new Response(conn->request->GetStatus(), Error);
			conn->state =  Connection::SENDING_RESPONSE;
		}
	}
	else if (conn->state == Connection::READING_REQUEST_LINE && conn->buffer.size() >= MAX_REQUEST_LINE_LENGHT)
	{
		conn->response = new Response(414, Error);
		SetSocketForWrite(conn);
		return;
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
			conn->state = Connection::PROCESSING;
		else
		{
			conn->response = new Response(conn->request->GetStatus(), Error);
			conn->state = Connection::SENDING_RESPONSE;
		}
	}
	else if (conn->state == Connection::READING_HEADERS && conn->buffer.size() >= MAX_header_field_LENGHT)
	{
		conn->response = new Response(431, Error); //[sessarhi] header field too large response
		SetSocketForWrite(conn);
		conn->state = Connection::SENDING_RESPONSE;
		return;
	}
}

void 		HttpServer::ProcessRequest(Connection *conn)
{
	if(!conn->request->CheckField("host"))
	{
		conn->response = new Response(400, Error);
        conn->state = Connection::SENDING_RESPONSE;
        return;
	}
	std::string host = conn->request->GetHeader("host");
	std::string hostname = host;
    size_t colon_pos = host.find(':');
    if (colon_pos != std::string::npos) 
	{
        hostname = host.substr(0, colon_pos);
    }
	struct sockaddr_in server_addr;
	socklen_t addr_len = sizeof(server_addr);
	if(getsockname(conn->fd, (struct sockaddr*)&server_addr, &addr_len) == -1)
		return;
	int port = ntohs(server_addr.sin_port);
	std::string ip = inet_ntoa(server_addr.sin_addr);
	Server* default_server = NULL;
    Server* matched_server = NULL;
    for (size_t i = 0; i < servers.size(); ++i) 
	{
        bool port_matches = false;
        for (size_t j = 0; j < servers[i].listen.size(); ++j) 
		{
            if (port == servers[i].listen[j].second && (servers[i].listen[j].first == ip || servers[i].listen[j].first == "0.0.0.0"))
			{
                port_matches = true;
                break;
            }
        }
        if (!port_matches) continue;
        if (!default_server) 
            default_server = &servers[i]; 
        for (size_t k = 0; k < servers[i].server_names.size(); ++k) {
            if (servers[i].server_names[k] == hostname) {
                matched_server = &servers[i];
                break;
            }
        }
        if (matched_server) break;
    }
    conn->server = matched_server ? matched_server : default_server;
    if (!conn->server) 
	{
        conn->response = new Response(500, Error);
        conn->state = Connection::SENDING_RESPONSE;
        return;
    }
	
	if(!MatchLocation(conn))
	{
		conn->response = new Response(404, Error);
		conn->state = Connection::SENDING_RESPONSE;
		return;
	}
	FillLocationMisseddata(conn);
	CheckCgiExist(conn); //added by eaboudi
	if (conn->request->GetMethod() == "POST")
	{
		if (conn->location->methods.find("POST") == conn->location->methods.end())
		{
			conn->response = new Response(405, Error);
			conn->state = Connection::SENDING_RESPONSE;
			return;
		}
		if (!conn->request->CheckField("content-type"))
		{
			conn->response = new Response(400, Error);
			conn->state = Connection::SENDING_RESPONSE;
			return;
		}
		if (!ProcessPostRequest(conn))
		{
			conn->state = Connection::SENDING_RESPONSE;
			return;
		}
		conn->response = new Response(conn->request->GetStatus(), POST);
	}
	else if (conn->request->GetMethod() == "GET")
	{
		if (conn->location->methods.find("GET") == conn->location->methods.end())
		{
			conn->response = new Response(405, Error);
			conn->state = Connection::SENDING_RESPONSE;
			return;
		}
		conn->response = new Response(conn->request->GetStatus(), GET);
	}
	else if (conn->request->GetMethod() == "DELETE")
	{
		if (conn->location->methods.find("DELETE") == conn->location->methods.end())
		{
			conn->response = new Response(405, Error);
			conn->state = Connection::SENDING_RESPONSE;
			return;
		}
		conn->response = new Response(conn->request->GetStatus(), DELETE);
	}
}
bool HttpServer::MatchLocation(Connection *conn)
{
    std::string target = conn->request->GetUri();
    size_t query_pos = target.find('?');
    if (query_pos != std::string::npos)
        target = target.substr(0, query_pos);
    std::map<std::string, LocationData>::iterator it = conn->server->locations.find(target);
    if (it != conn->server->locations.end()) {
        conn->location = &it->second;
        return true;
    }
    while (!target.empty()) 
	{
        size_t last_slash = target.find_last_of('/');
        if (last_slash == std::string::npos)
            break;
        if (last_slash == 0)
            target = "/";
        else
            target = target.substr(0, last_slash);
        it = conn->server->locations.find(target);
        if (it != conn->server->locations.end())
		{
            conn->location = &it->second;
            return true;
        }
        if (target == "/")
            break;
    }
    it = conn->server->locations.find("/");
    if (it != conn->server->locations.end())
	{
        conn->location = &it->second;
        return true;
    }
    return false;
}


bool		HttpServer::ProcessPostRequest(Connection *conn)
{
	if (conn->location->methods.find("POST") == conn->location->methods.end())
	{
		conn->response = new Response(405 , Error);
		return false;
	}
	if (!conn->location->upload)
	{
		conn->response = new Response(403 , Error);
		return false;
	}
	if (conn->request->CheckField("transfer-encoding"))
	{
		std::string encoding = conn->request->GetHeader("transfer-encoding");
		std::vector<std::string> tokens;
		std::stringstream ss(encoding);
		std::string token;
		while (std::getline(ss, token, ','))
		{
			Request::trim(token);
			size_t semicolon = token.find(';');
			if (semicolon != std::string::npos)
				token = token.substr(0, semicolon);
			tokens.push_back(token);
		}
		bool chunked_found = false;
		for (size_t i = 0; i < tokens.size(); ++i) 
		{
			if (tokens[i] == "chunked") 
			{
				if (chunked_found || i != tokens.size() - 1)
				{
					conn->response = new Response(400, Error);
					return false;
				}
				chunked_found = true;
			}
		}
		if (!chunked_found) 
		{
			conn->response = new Response(400, Error);
			return false;
		}
		conn->post = new Post(conn, Post::CHUNKED);
		return true;
	}
	if (!conn->request->GetHeader("content-length").empty())
		conn->post = new Post(conn,Post::CONTENT_LENGTH);
	else
	{
		conn->response = new Response(411 , Error);
		return false;
	}
	return true;
}

void 		HttpServer::FillLocationMisseddata(Connection *conn)
{
	if (conn->location->root.empty())
		conn->location->root = conn->server->root;
	if (conn->location->index.empty() && !conn->server->index.empty())
		conn->location->index = conn->server->index;
	if (!conn->location->has_redirect && conn->server->has_redirect)
	{
		conn->location->has_redirect = conn->server->has_redirect;
		conn->location->redirect.first = conn->server->redirect.first;
		conn->location->redirect.second = conn->server->redirect.second;
	}
	if (!conn->location->autoindex_set && conn->server->autoindex_set)
	{
		conn->location->autoindex = conn->server->autoindex;
		conn->location->autoindex_set = true;
	}
	if (conn->location->cgi.empty() && !conn->server->cgi.empty())
	{
		for (std::map<std::string,std::string>::iterator it = conn->server->cgi.begin();it != conn->server->cgi.end();++it)
		{
			if (conn->location->cgi.find((*it).first) != conn->location->cgi.end())
				conn->location->cgi[(*it).first] = (*it).second;
		}
	}
	if (!conn->location->upload && conn->server->upload)
	{
		conn->location->upload = conn->server->upload;
		conn->location->upload_store = conn->server->upload_store;
	}
	if (conn->location->error_pages.empty() && !conn->server->error_pages.empty())
		conn->location->error_pages = conn->server->error_pages;
	if (conn->location->max_body_size == 0 && conn->server->max_body_size > 0)
		conn->location->max_body_size = conn->server->max_body_size;
	if (!conn->location->upload_store.empty() && *(--conn->location->upload_store.end()) != '/')
		conn->location->upload_store.push_back('/');
}
