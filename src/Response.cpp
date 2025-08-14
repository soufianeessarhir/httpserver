/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:32:24 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/13 22:10:55 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

const std::map<int, std::string> Response::ErrorPhrase = createErrorPhrase();
std::map<int, std::string> createErrorPhrase()
{
    std::map<int, std::string> m;
    m[400] = "Bad Request";
    m[401] = "Unauthorized";
    m[403] = "Forbidden";
    m[404] = "Not Found";
    m[405] = "Method Not Allowed";
    m[406] = "Not Acceptable";
    m[408] = "Request Timeout";
    m[409] = "Conflict";
    // m[410] = "Gone";
    m[411] = "Length Required";
    // m[412] = "Precondition Failed";
    m[413] = "Payload Too Large";
    m[414] = "URI Too Long";
    m[415] = "Unsupported Media Type";
    // m[416] = "Range Not Satisfiable";
    // m[417] = "Expectation Failed";
    // m[418] = "I'm a teapot";
    // m[421] = "Misdirected Request";
    // m[422] = "Unprocessable Entity";
    // m[423] = "Locked";
    // m[424] = "Failed Dependency";
    // m[425] = "Too Early";
    // m[426] = "Upgrade Required";
    // m[428] = "Precondition Required";
    m[429] = "Too Many Requests";
    m[431] = "Request Header Fields Too Large";
    m[451] = "Unavailable For Legal Reasons";
    m[500] = "Internal Server Error";
    m[501] = "Not Implemented";
    m[502] = "Bad Gateway";
    m[503] = "Service Unavailable";
    m[504] = "Gateway Timeout";
    m[505] = "HTTP Version Not Supported";
    m[506] = "Variant Also Negotiates";
    m[507] = "Insufficient Storage";
    m[508] = "Loop Detected";
    m[510] = "Not Extended";
    m[511] = "Network Authentication Required";
    return m;
}

void    Response::SetStatusCode(int New)
{
    StatusCode = New;
}

Response::Response() : GET(NULL), Error(NULL), DELETE(NULL), POST(NULL) ,StatusCode(0)
{
    
}

Response::Response(int errorCode, Methods _Method) : GET(NULL), Error(NULL), DELETE(NULL), POST(NULL) ,Method(_Method), StatusCode(errorCode)
{
    
}

Response::~Response()
{
    if (GET)
        delete GET;
    if (DELETE)
        delete DELETE;
    if (Error)
        delete Error;
    if (POST)
        delete POST;

    GET = NULL;
    DELETE = NULL;
    Error = NULL;
    POST = NULL;
}

int Response::GetStatusCode() const
{
    return StatusCode;
}

Methods Response::GetMethod() const
{
    return Method;
}

void Response::SetMethod(Methods method)
{
    Method = method;
}

