/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultiPart.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:17:24 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/02 17:24:31 by sessarhi         ###   ########.fr       */
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
        std::string name;
        std::string filename;
        bool is_file_upload;
        bool ProcessMultiPartHeaders(std::string data);
        bool ConfigureMultipart();
};

#endif