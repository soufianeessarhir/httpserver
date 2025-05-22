/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:30:53 by sessarhi          #+#    #+#             */
/*   Updated: 2025/05/22 21:46:26 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Request.hpp"

std::string Request::GetUri()const {return uri;}

const char* Request::GetBody()const{return body;}

std::string Request::GetHeader(std::string name)
{
    std::map<std::string, std::string>::iterator it = headers.find(name);
    if (it != headers.end())
        return (it->second);
    return NULL;
}

std::string Request::GetVersion()const
{
    return version;
}

void        Request::SetBody(std::string &data)
{
    body = data.c_str();
}

bool        Request::ParseHeaders(std::string data)
{
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss,line))
    {
        if (line.empty())
        {
            IsComplete = true;
            return true;
        }
        size_t del = line.find(':');
        if(del ==  std::string::npos)
        {
            RequestStatusCode = 400;
            return false;
        }
        std::string name = line.substr(0,del);
        std::string value = line.substr(del + 1);
        trim(name);
        trim(value);
        ToCanonical(name);
        headers[name] = value;
    }
    return true;
    
}

bool        Request::ParseRequestLine(std::string data)
{
    std::istringstream iss(data);
    if (!(iss >> method >> uri >> version))
    {
        // [sessarhi] bad request
        RequestStatusCode = 400;
        return false;
        
    }
    if (method != "GET" && method != "POST" && method != "DELETE")
    {
        // [sessarhi] not implemented
        RequestStatusCode = 501;
        return false;
        
    }
    if (version != "HTTP/1.1" && version != "HTTP/1.0")
    {
        // [sessarhi] not supported
        RequestStatusCode = 505;
        return false;
    }
    if (ParseUri())
        return false;
        
    return true;
}

bool        Request::ParseUri()
{
    const std::string Ilegall = "<>\"\\^`{}|";
    if (uri.empty() || uri[0] != '/')
    {
        RequestStatusCode = 400;
        return false;
    }
    for (std::string::iterator it = uri.begin(); it != uri.end(); it++)
    {
        if (*it == 127 || (*it >= 0 && *it <= 31) ||  isspace(static_cast<unsigned char>(*it)) || Ilegall.find(*it) != std::string::npos)
        {
            RequestStatusCode = 400;
            return false;
        }
    }
    bool ret = Decode();
    if (ret)
        return false;
    

    return true;
}

bool        Request::ExpectBody()const
{
    return method == "POST";
}

bool        Request::KeepAlive()const
{
    return true;
}

void        Request::ToCanonical(std::string &str)
{
    for (std::string::iterator it = str.begin(); it != str.end();++it)
    {
        *it = static_cast<char>(std::tolower(*it));
    }
}

bool        Request::Decode()
{
    const std::string tmp = uri;

    uri.clear();
    for (size_t i = 0; i < tmp.size();++i)
    {
        if (tmp[i] == '%')
        {
            if (i + 2 >= tmp.size() || !isxdigit(tmp[i + 1]) || !isxdigit(tmp[i + 2]))
            {
                RequestStatusCode = 400;
                return false;
            }
            std::istringstream os(tmp.substr(i +1 ,2));
            int val = 0;
            os >> std::hex >> val;
            uri.push_back(static_cast<char>(val));
            i=+2;
        }
        else
            uri.push_back(tmp[i]);
    }
    //[sessarhi] may i need to check for illegat characters after decoding   
    return true;
}


void        Request::trim(std::string &str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start])))
        ++start;

    size_t end = str.size();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1])))
        --end;

    str = str.substr(start, end - start);
}
