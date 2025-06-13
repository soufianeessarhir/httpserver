/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 14:50:50 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/13 17:10:56 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef         CONNECTION_HPP
#define         CONNECTION_HPP

#include "Connection.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "ConfigData.hpp"

class Request;
class Response;
class Server;
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
    LocationData    *Location;
    std::string     buffer;
    __ssize_t       BytesSent;
    time_t          LastAct;
};

#endif