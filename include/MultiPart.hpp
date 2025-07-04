/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MultiPart.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/25 11:17:24 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/04 16:56:20 by sessarhi         ###   ########.fr       */
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
	std::string file;
	MultiPart(std::string & rfile):file(rfile){}
	~MultiPart(){
	}
};

#endif