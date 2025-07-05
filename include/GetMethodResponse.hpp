/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetMethodResponse.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 11:23:23 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/05 12:27:17 by sessarhi         ###   ########.fr       */
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
        GetMethodResponse(int statusCode, std::string filePath);
        ~GetMethodResponse();
    
        void SetHeaders();
        void SetBody();
        void SetContentType();
        void SetStatusLine();

        const std::string& GetBody() const;
        const std::string& GetContentType() const;
        int GetStatusCode() const;

        void    SendStatusLine(Connection *Conn);
        void    SendHeaders(Connection *Conn);
        void    SendBody(Connection *Conn);
};


std::map<std::string, std::string> CreateMimeTypes();
void    excuteGetMethod(Connection *conn);

#endif