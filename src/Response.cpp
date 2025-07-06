/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:32:24 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/06 11:14:49 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Response.hpp"

const std::map<int, std::string> Response::ErrorPhrase = createErrorPhrase();
std::map<int, std::string> createErrorPhrase()
{
    std::map<int, std::string> m;
    m[400] = "Bad Request";
    m[401] = "Unauthorized";
    m[402] = "Payment Required";
    m[403] = "Forbidden";
    m[404] = "Not Found";
    m[405] = "Method Not Allowed";
    m[406] = "Not Acceptable";
    m[407] = "Proxy Authentication Required";
    m[408] = "Request Timeout";
    m[409] = "Conflict";
    m[410] = "Gone";
    m[411] = "Length Required";
    m[412] = "Precondition Failed";
    m[413] = "Payload Too Large";
    m[414] = "URI Too Long";
    m[415] = "Unsupported Media Type";
    m[416] = "Range Not Satisfiable";
    m[417] = "Expectation Failed";
    m[418] = "I'm a teapot";
    m[421] = "Misdirected Request";
    m[422] = "Unprocessable Entity";
    m[423] = "Locked";
    m[424] = "Failed Dependency";
    m[425] = "Too Early";
    m[426] = "Upgrade Required";
    m[428] = "Precondition Required";
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

Response::Response() : GET(NULL), StatusCode(0)
{
    
}

Response::Response(int errorCode, Methods _Method) : GET(NULL), Method(_Method), StatusCode(errorCode)
{
    
}

Response::Response(Request *req,Server *srv)
{
    (void)req;   // [sessarhi] should be updated to generate respons from server and the request
    
    (void)srv;
}

Response::~Response()
{
    
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

void Response::ErrorResponse(Connection *Conn)
{
    // Build the status line
    std::stringstream BuildStatusLine;
    BuildStatusLine << "HTTP/1.1 ";
    std::map<int, std::string>::const_iterator it = ErrorPhrase.find(StatusCode);
    if (it != ErrorPhrase.end())
        BuildStatusLine << StatusCode << " " << it->second << "\r\n";
    else
    {
        BuildStatusLine << "500 Internal Server Error\r\n";
        StatusCode = 500;
    }
    StatusLineError = BuildStatusLine.str();
    std::string Headers = "Content-Type: text/html\r\n";
    Headers += "Content-Length: 0\r\n\r\n";
    // Headers += "Connection: close\r\n\r\n";
    std::string ResponseData = StatusLineError + Headers;
    ssize_t BytesSent = send(Conn->fd, ResponseData.c_str(), ResponseData.size(), MSG_DONTWAIT);
    if (BytesSent < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        else
        {
            perror("send");
            Conn->state = Connection::COMPLETE; // Mark connection as complete on error
            return;
        }
    }   
}
