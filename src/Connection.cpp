/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:32:49 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/19 10:37:36 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Connection.hpp"

Connection::Connection(int fd): state(READING_REQUEST_LINE),fd(fd)
            ,request(new Request()),response(NULL),server(NULL),location(NULL),post(NULL),BytesSent(0),LastAct(time(NULL))
{
            
}


