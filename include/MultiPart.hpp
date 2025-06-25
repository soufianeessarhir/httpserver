/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultiPart.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:17:24 by sessarhi          #+#    #+#             */
/*   Updated: 2025/06/25 11:20:10 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPART_HPP
#define MULTIPART_HPP

#include <string>
#include <map>

class MultipartPart 
{
    private:
        std::map<std::string, std::string> headers;
        std::string content;
        std::string name;
        std::string filename;
        std::string contentType;
        
    public:
        bool isFile() const;
        std::string getName() const;
        std::string getFilename() const;
        std::string getContent() const;
        void saveToFile(const std::string& path) const;
};
#endif