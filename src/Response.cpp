/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:32:24 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/12 12:19:28 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

// Response::Response(int errorCode)
// {
    
// }

Response::Response(Request *req,Server *srv)
{
    (void)req;   // [sessarhi] should be updated to generate respons from server and the request
    
    (void)srv;
}

Response::~Response()
{
    
}
