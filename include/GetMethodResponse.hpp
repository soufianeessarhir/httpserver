/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   GetMethodResponse.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 11:23:23 by eaboudi           #+#    #+#             */
/*   Updated: 2025/06/12 11:48:16 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef GETMETHODRESPONSE_HPP
#define GETMETHODRESPONSE_HPP


// #include "Response.hpp"
#include <fstream>
#include "BaseResponse.hpp"


class GetMethodResponse : public BaseResponse
{
    private:
    std::string     FilePath;
    static const    std::map<std::string, std::string>  MimeTypes;
    bool            IsBinaryFile;
    public:
    GetMethodResponse(int statusCode, std::string filePath);
    ~GetMethodResponse();
    
    void SetHeaders();
    void SetBody();
    void SetContentType();
};

std::map<std::string, std::string> CreateMimeTypes();
#endif