/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:30:12 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/11 10:09:04 by sessarhi         ###   ########.fr       */
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

    Request():RequestStatusCode(0) ,IsComplete(false){}
    
    ~Request(){}
    
    bool            ParseHeaders(std::string );
    
    bool            ParseRequestLine(std::string& );
    
    bool            ExpectBody()const;
    
    size_t          GetContentLenght()const;
    
    
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

    std::string     GetHeader(std::string )const;
    
    std::string     GetVersion()const;

    int             GetStatus()const;
    
    bool            GetIsComplet()const;

    std::string    GetMethod();

    bool            CheckField(const std::string &);
    

private:

    int                                 RequestStatusCode;

    bool                                IsComplete;
    
    std::string                         method;
    
    std::string                         uri;
    
    std::string                         version;
    
    std::map<std::string,std::string>   headers;

};


#endif