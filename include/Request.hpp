/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/11 15:30:12 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/03 11:37:28 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef             REQUEST_HPP

#define             REQUEST_HPP

#include            <string>

#include            <map>

#include            <sstream>

#include            <errno.h> 

#include            <cstdlib>


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
    
    void            ToCanonical(std::string &);

    void            trim(std::string &);

    bool            ParseUri();

    bool            Decode();

    bool            OnlySpaces(std::string&);

    bool            Haswhitespace(std::string&);

    
    std::string     GetUri()const;

    const char*     GetBody()const;
    
    std::string     GetHeader(std::string )const;
    
    std::string     GetVersion()const;

    int             GetStatus()const;
    
    bool            GetIsComplet()const;
    

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