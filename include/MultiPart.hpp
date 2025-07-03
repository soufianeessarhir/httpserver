/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultiPart.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:17:24 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/03 17:30:11 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MULTIPART_HPP
#define MULTIPART_HPP

#include <string>
#include <map>
#include <fstream>
#include <sstream>
#include "Request.hpp"

struct  MultiPart 
{
        std::map<std::string, std::string> headers;
        std::string name;
        std::string filename;
        std::ofstream *output_file;
        bool is_file_upload;
        bool ProcessMultiPartHeaders(std::string data);
        bool ConfigureMultipart();
		MultiPart():output_file(NULL){}
		~MultiPart(){
			if (output_file)
			{
				output_file->close();
				delete output_file;
				output_file = NULL;
			}
		}
};

#endif