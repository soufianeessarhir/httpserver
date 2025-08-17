/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 14:50:50 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/17 11:42:30 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef         CONNECTION_HPP
#define         CONNECTION_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "ConfigData.hpp"
#include "Post.hpp"
#include <string>
#include <map>

class Request;
class Response;
class Post;
class CGI;
struct Timeouts
{
    bool read_fails;
	time_t last_activity; // Last activity timestamp
    time_t read_timeout;  // Read timeout value
    time_t Child_track; // cgi process
};

class Connection
{

public:
    Connection(int fd, int event);
    void Reset();
    void UpdateTime(time_t &t);
    ~Connection();
    int event_fd;
    enum State 
    {
        READING_REQUEST_LINE,
        READING_HEADERS,
        READING_BODY,
        PROCESSING,
        SENDING_RESPONSE,
        COMPLETE
    };
    bool            UseCgi;
    CGI             *CgiObj;
    bool            Cookies;
    std::string     CookiesString;
    Timeouts        timeouts;
    State           state;
    int             fd;
    Request         *request;
    Response        *response;
    Server          *server;
    LocationData    *location;
    Post            *post;
    std::string     buffer;
    
    std::string     ip;
    int             port;
};

void    CheckCgiExist(Connection *conn);

#endif