/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:32:24 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/11 16:32:21 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

// Response::Response(int errorCode)
// {
    
// }

Response::Response(Request &req,Server &srv)
{
    (void)req;   // [sessarhi] should be updated to generate respons from server and the request
    
    (void)srv;
}

Response::~Response()
{
    
}
