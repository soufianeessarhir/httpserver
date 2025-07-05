/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:30:53 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/05 16:51:39 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Request.hpp"

std::string Request::GetUri() {return uri;}

const char* Request::GetBody()const{return body;}

std::string Request::GetHeader(std::string name)const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(name);
    if (it != headers.end())
        return (it->second);
    return "";
}
size_t          Request::GetContentLenght()const
{
    errno = 0;
    char *endptr;
    std::map<std::string,std::string>::const_iterator it = headers.find("contentlenght");
    if (it == headers.end())
        return 0;
    long long val =  std::strtol((*it).second.c_str(),&endptr,10);
    return val;
    
}

bool Request::CheckField(const std::string &field)
{
    std::map<std::string, std::string>::const_iterator it = headers.find(field);
    if (it != headers.end())
        return (true);
    return false;
}

int         Request::GetStatus()const
{
    return RequestStatusCode;
}

std::string Request::GetVersion()const
{
    return version;
}

bool        Request::GetIsComplet()const
{
    return IsComplete;
}

std::string     Request::GetMethod()
{
    return method;
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
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        if (headers.size() == 0)
        {
            if (line.empty() || OnlySpaces(line))
            {
                RequestStatusCode = 400;
                return false;
            }
        }
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
        if (name.empty() || Haswhitespace(name)) //[sessarhi] maybe i need to check for emply fileds | values
        {
            RequestStatusCode = 400;
            return false;
        }
        std::string value = line.substr(del + 1);
        trim(value);
        ToCanonical(name);
        headers[name] = value;
    }
    return true;
    
}

bool        Request::OnlySpaces(std::string &line)
{
    for (size_t i = 0;i < line.size();++i)
    {
        if (!isspace(static_cast<unsigned char>(line[i])))
            return false;
    }
    return true;
}

bool        Request::ParseRequestLine(std::string& data)
{
    
    size_t first_space = data.find(' ');
    if (first_space == std::string::npos)
    {
        RequestStatusCode = 400;
        return false;
    }
    
    size_t second_space = data.find(' ',first_space + 1);
    if (second_space == std::string::npos)
    {
        RequestStatusCode = 400;
        return false;
    }
    size_t invalid = data.find(' ', second_space + 1);
    if (invalid != std::string::npos)
    {
        RequestStatusCode = 400;
        return false;
    }
    method = data.substr(0,first_space);
    uri = data.substr(first_space + 1, second_space - first_space - 1);
    version = data.substr(second_space + 1);
    if (method.empty() || uri.empty() || version.empty())
    {
        RequestStatusCode = 400;
        return false;
    }
    if (method != "GET" && method != "POST" && method != "DELETE")
    {
        // [sessarhi] not implemented
        RequestStatusCode = 501;
        return false;
        
    }
    if (version != "HTTP/1.1")
    {
        // [sessarhi] not supported
        RequestStatusCode = 505;
        return false;
    }
    if (!ParseUri())
        return false; 
    return true;
    
}

bool            Request::Haswhitespace(std::string& FieldName)
{
    if (isspace(FieldName[0]) || isspace(FieldName[FieldName.size() - 1]))
        return true;
    return false;
}       

void            Request::RemoveFromOutputpath(std::string &output)
{
    if (output.empty())
        return;
    size_t last_slash = output.find_last_of('/');
    if (last_slash == std::string::npos)
    {
        output.clear();
        return;
    }
    output = output.substr(0,last_slash);
}
void            Request::NormalizePath()
{
    size_t querypos = uri .find('?');
    std::string query;
    std::string output;
    if(querypos != std::string::npos)
    {
        query = uri .substr(querypos);
        uri  = uri .substr(0,querypos);
    }
    else
        query = "";
    while(!uri .empty())
    {
        if(uri .length() >= 3 && uri .substr(0,3) == "../")// ../
        {
            uri  = uri .substr(3);
        }
        else if(uri .length() >= 2 && uri .substr(0,2) == "./") // ./
        {
            uri  = uri .substr(2);
        }
        else if(uri .length() >= 3 && uri .substr(0,3) == "/./") // /./
        {
            uri  = "/" +  uri .substr(3);
        }
        else if(uri  == "/.") // /.
        {
            uri  = "/";
        }
        else if(uri .length() >= 4 && uri .substr(0,4) == "/../") // /../
        {
            uri  = uri .substr(3);
            RemoveFromOutputpath(output);
        }
        else if(uri  == "/..") // /..
        {
            uri  = "/";
            RemoveFromOutputpath (output);
        }
        else if (uri  == "." || uri  == "..")
        {
            uri  = "";
        }
        else
        {
            size_t first_slash = uri .find('/',1);
            if (first_slash == std::string::npos && !uri .empty())
            {
                output += uri ;
                uri  = "";
            }
            else
            {
                output += uri .substr(0,first_slash);
                uri  = uri .substr(first_slash);
            }
        }
    }
    uri = output + query;
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
    if (!Decode())
        return false;
    // [sessarhi] may i add checking for utf-8 sequences
    NormalizePath();
    if (uri.empty() || uri[0] != '/')
        return false;
    return true;
}

bool        Request::ExpectBody()const
{
    return headers.find("content-length") != headers.end()
        || headers.find("transfer-encoding") != headers.end();
    //because presence of body is depends on the content 
    //length or transfer encoding only
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

