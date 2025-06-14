/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   BaseResponse.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/11 11:40:30 by eaboudi           #+#    #+#             */
/*   Updated: 2025/06/14 10:35:56 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BASE_RESPONSE_HPP
#define BASE_RESPONSE_HPP


#include "Connection.hpp"


class Connection;

class BaseResponse
{
    protected:
        int             StatusCode;
        std::string     StatusLine;
        std::string     Body;
        size_t          ContentLength;
        std::string     ContentType;
        std::map<std::string, std::string> Headers;
        static const std::map<int, std::string> ErrorPhrase;
        ssize_t       BytesSent;

    public:
        BaseResponse();
        BaseResponse(int statusCode);
        virtual ~BaseResponse();

        virtual void SetHeaders() = 0;
        virtual void SetBody() = 0;
        virtual void SetContentType() = 0;
        void SetStatusLine();

        const std::string& GetBody() const;
        const std::string& GetContentType() const;
        int GetStatusCode() const;

        void    SendStatusLine(Connection *Conn);
        void    SendHeaders(Connection *Conn);
        void    SendBody(Connection *Conn);
};

std::map<int, std::string> createErrorPhrase();

#endif