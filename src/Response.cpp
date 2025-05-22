/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:32:24 by sessarhi          #+#    #+#             */
/*   Updated: 2025/05/15 11:29:49 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Response.hpp"

Response::Response(int status):StatusCode(status)
{
    
}

Response::Response(Request &req,Server &srv)
{
    (void)req;   // [sessarhi] should be updated to generate respons from server and the request
    
    (void)srv;
}

Response::~Response()
{
    
}

int Response::GetStatus()const{return StatusCode;}

std::string Response::GetData()const {return data;}

void Response::BuildResponse()
{
    
}