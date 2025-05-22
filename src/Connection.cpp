/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Connection.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:32:49 by sessarhi          #+#    #+#             */
/*   Updated: 2025/05/19 15:23:10 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Connection.hpp"

Connection::Connection(int fd): state(READING_REQUEST_LINE),fd(fd)
            ,request(new Request()), response(NULL),BytesSent(0),LastAct(time(NULL))
{
            
}


