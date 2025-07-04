/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetMethodResponse.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 11:23:23 by eaboudi           #+#    #+#             */
/*   Updated: 2025/06/29 11:42:01 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GETMETHODRESPONSE_HPP
#define GETMETHODRESPONSE_HPP


// #include "Response.hpp"
#include <fstream>

#include "Connection.hpp"


class Connection;
class GetMethodResponse
{
    private:
        int             StatusCode;
        std::string     StatusLine;
        std::string     Body;
        size_t          ContentLength;
        std::string     ContentType;
        std::map<std::string, std::string> Headers;
        static const std::map<int, std::string> ErrorPhrase;
        ssize_t       BytesSent;
        std::string     FilePath;
        static const    std::map<std::string, std::string>  MimeTypes;
        bool            IsBinaryFile;
    public:
        GetMethodResponse(int statusCode, std::string filePath);//ok
        ~GetMethodResponse();//ok
    
        void SetHeaders();//ok
        void SetBody();//ok
        void SetContentType(); //ok
        void SetStatusLine();

        const std::string& GetBody() const;//ok
        const std::string& GetContentType() const;//ok
        int GetStatusCode() const;//ok

        void    SendStatusLine(Connection *Conn);//ok
        void    SendHeaders(Connection *Conn);//ok
        void    SendBody(Connection *Conn);//ok
};


std::map<std::string, std::string> CreateMimeTypes();


#endif