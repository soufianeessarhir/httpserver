/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 14:50:50 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/29 12:03:14 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef         CONNECTION_HPP
#define         CONNECTION_HPP

#include "Request.hpp"
#include "Response.hpp"
#include "ConfigData.hpp"
#include "Post.hpp"

class Request;
class Response;
class Server;
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
    
    State           state;
    int             fd;
    Request         *request;
    Response        *response;
    Server          *server;
    LocationData    *location;
    Post            *post;
    std::string     buffer;
    __ssize_t       BytesSent;
    time_t          LastAct;
};

#endif