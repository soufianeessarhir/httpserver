/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 14:50:50 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/10 19:24:22 by sessarhi         ###   ########.fr       */
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
#define			READ_BUFFER_SIZE			64000

class Request;
class Response;
class Post;
class CGI;
struct Timeouts
{
    bool read_fails;
	time_t last_activity; // Last activity timestamp
    time_t read_timeout;  // Read timeout value
};

class Connection
{

public:
    Connection(int fd);
    void Reset();
    void UpdateTime(time_t &t);
    ~Connection();
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
    char buf[READ_BUFFER_SIZE];
    std::string     ip;
    int             port;
};

void    CheckCgiExist(Connection *conn);

#endif