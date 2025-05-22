/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 14:50:50 by sessarhi          #+#    #+#             */
/*   Updated: 2025/05/22 18:54:41 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef         CONNECTION_HPP
#define         CONNECTION_HPP

#include        "Request.hpp"
#include        "Response.hpp"

class Connection
{

public:
    Connection(int fd);
    
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
    std::string     buffer;
    __ssize_t       BytesSent;
    time_t          LastAct;
};

#endif