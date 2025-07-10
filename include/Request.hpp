/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:30:12 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/10 08:51:22 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef             REQUEST_HPP

#define             REQUEST_HPP

#include            <string>

#include            <map>

#include            <sstream>

#include            <errno.h> 

#include            <cstdlib>

#include            <iostream>

#include            "CGI.hpp" // added by eaboudi

#define CGiDir "/home/eaboudi/Desktop/httpserver/bin" // added by eaboudi
class Request
{

public:

    Request():IsComplete(false), body(NULL){}
    
    ~Request(){}
    
    bool            ParseHeaders(std::string );
    
    bool            ParseRequestLine(std::string& );
    
    bool            ExpectBody()const;
    
    size_t          GetContentLenght()const;
    
    void            SetBody(std::string &);
    
    bool            KeepAlive()const;
    
    static void     ToCanonical(std::string &);

    static void     trim(std::string &);

    bool            ParseUri();

    bool            Decode();

    static bool     OnlySpaces(std::string&);

    static bool     Haswhitespace(std::string&);

    void            NormalizePath();

    void            RemoveFromOutputpath(std::string &);

    void            CheckCgiExist(); //add by eaboudi

    bool            UseCgi; // added by eaboudi

    CGI             *CgiObj; // added by eaboudi

    



    
    std::string     GetUri();

    const char*     GetBody()const;
    
    std::string     GetHeader(std::string )const;
    
    std::string     GetVersion()const;

    int             GetStatus()const;
    
    bool            GetIsComplet()const;

    std::string    GetMethod();

    bool            CheckField(const std::string &);
    

private:

    int                                 RequestStatusCode;

    bool                                IsComplete;

    const char*                         body;
    
    std::string                         method;
    
    std::string                         uri;
    
    std::string                         version;
    
    std::map<std::string,std::string>   headers;

};


#endif