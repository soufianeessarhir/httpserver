/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:32:49 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/05 16:49:36 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

Connection::Connection(int fd): state(READING_REQUEST_LINE),fd(fd)
            ,request(new Request()),response(NULL),server(NULL),location(NULL),post(NULL),BytesSent(0),LastAct(time(NULL))
{
            
}


