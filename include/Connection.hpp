/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 14:50:50 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/21 09:47:45 by eaboudi          ###   ########.fr       */
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


class Connection
{

public:
    Connection(int fd);
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
    
    State           state;
    int             fd;
    Request         *request;
    Response        *response;
    Server          *server;
    LocationData    *location;
    Post            *post;
    std::string     buffer;
    time_t          LastAct;
    std::string     ip;
    int             port;
};

void    CheckCgiExist(Connection *conn);

#endif