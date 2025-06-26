/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultiPart.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:17:24 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/26 11:37:11 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPART_HPP
#define MULTIPART_HPP

#include <string>
#include <map>
#include <sstream>
#include "Request.hpp"

struct  MultiPart 
{
        std::map<std::string, std::string> headers;
        std::string content;
        std::string name;
        std::string filename;
        std::string contentType;
        bool ProcessMultiPartHeaders(std::string data);
        bool isFile() const;
        std::string getName() const;
        std::string getFilename() const;
        std::string getContent() const;
        void saveToFile(const std::string& path) const;
};
#endif