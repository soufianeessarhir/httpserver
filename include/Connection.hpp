/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 14:50:50 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/01 12:19:00 by sessarhi         ###   ########.fr       */
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
// class Server;
class Post;
class CGI;

struct Timeouts
{
    bool read_fails;
	time_t last_activity; // Last activity timestamp
    time_t read_timeout;  // Read timeout value
    // time_t write_timeout; // Write timeout value
    time_t idle_timeout;
    void UpdateLastActivityTime(){last_activity = time(NULL);}
    void UpdateIdleTime(){idle_timeout = time(NULL);}
};

class Connection
{

public:
    Connection(int fd);
    void Reset();
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
    bool            UseCgi; // added by eaboudi
    CGI             *CgiObj; // added by eaboudi
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