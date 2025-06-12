/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BaseResponse.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 11:46:29 by eaboudi           #+#    #+#             */
/*   Updated: 2025/06/11 23:10:15 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "BaseResponse.hpp"
#include "Response.hpp"
#include "HttpServer.hpp"

class BaseResponse;

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

BaseResponse::BaseResponse() : StatusCode(200), ContentLength(0) 
{

}

BaseResponse::BaseResponse(int statusCode) : StatusCode(statusCode), ContentLength(0) 
{

}

BaseResponse::~BaseResponse() 
{

}

const std::string& BaseResponse::GetBody() const
{
    return Body;
}
const std::string& BaseResponse::GetContentType() const
{
    return ContentType;
}

int BaseResponse::GetStatusCode() const
{
    return StatusCode;
}

void BaseResponse::SetStatusLine()
{
    std::stringstream BuildStatusLine(HttpVersion);
    std::map<int, std::string>::const_iterator it = ErrorPhrase.find(StatusCode);
    if (it != ErrorPhrase.end())
        BuildStatusLine << StatusCode << " " << it->second << "\r\n";
    else
        BuildStatusLine << StatusCode << " OK\r\n";
    StatusLine = BuildStatusLine.str();
    
}

const std::map<int, std::string> BaseResponse::ErrorPhrase = createErrorPhrase();

void BaseResponse::SendStatusLine(Connection *Conn)
{
    ssize_t BytesWriten = 0;
    while (BytesSent < static_cast<ssize_t>(StatusLine.size()))
    {
        BytesWriten = send(Conn->fd, StatusLine.c_str() + BytesSent, 
                            StatusLine.size() - BytesSent, 0);
        if (BytesWriten < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                Conn->state = Connection::SENDING_RESPONSE;
                return; // Wait for next round
            }
            else
            {
                perror("send");
                Conn->state = Connection::COMPLETE; // Error handling
                return;
            }
        }
        else
            BytesSent += BytesWriten;
        Conn->BytesSent += BytesSent;
        if (BytesSent == 0)
        {
            Conn->state = Connection::COMPLETE; // No more data to send
            return;
        }
    }
    if (BytesSent >= static_cast<ssize_t>(StatusLine.size()))
        BytesSent = 0;
}

void BaseResponse::SendHeaders(Connection *Conn)
{
    ssize_t BytesWriten = 0;
    std::string HeadersStr;
    std::map<std::string, std::string>::const_iterator it;
    for (it = Headers.begin(); it != Headers.end(); ++it)
        HeadersStr += it->first + ": " + it->second;
    HeadersStr += "\r\n";
    while (BytesSent < static_cast<ssize_t>(HeadersStr.size()))
    {
        BytesWriten = send(Conn->fd, HeadersStr.c_str() + BytesSent, 
                            HeadersStr.size() - BytesSent, 0);
        if (BytesWriten < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                Conn->state = Connection::SENDING_RESPONSE;
                return; // Wait for next round
            }
            else
            {
                perror("send");
                Conn->state = Connection::COMPLETE; // Error handling
                return;
            }
        }
        else
            BytesSent += BytesWriten;
    }
    if (BytesSent >= static_cast<ssize_t>(HeadersStr.size()))
        BytesSent = 0;
}

void BaseResponse::SendBody(Connection *Conn)
{
    ssize_t BytesWriten = 0;
    while (BytesSent < static_cast<ssize_t>(Body.size()))
    {
        BytesWriten = send(Conn->fd, Body.c_str() + BytesSent, 
                            Body.size() - BytesSent, 0);
        if (BytesWriten < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                Conn->state = Connection::SENDING_RESPONSE;
                return; // Wait for next round
            }
            else
            {
                perror("send");
                Conn->state = Connection::COMPLETE; // Error handling
                return;
            }
        }
        else
            BytesSent += BytesWriten;
    }
    if (BytesSent >= static_cast<ssize_t>(Body.size()))
        BytesSent = 0;
    
}
