/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigData.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:14:10 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/10 19:33:42 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CONFIGDATA_HPP
# define CONFIGDATA_HPP
#include <string>
#include <vector>
#include <map>
#include <set>

struct CommonConfig 
{
    std::string root;
    std::string index;
    bool has_redirect;
    std::pair<int, std::string> redirect;
    bool autoindex_set /*= false*/;
    bool autoindex;
    std::map<std::string, std::string> cgi;
    // bool upload_set /*= false*/;
    bool upload;
    std::string upload_store;
	std::map<int,std::string> error_pages;
    size_t max_body_size /*0*/;
    CommonConfig():root(""),index(""),has_redirect(false),
    autoindex_set(false),upload(false),upload_store(""),
    max_body_size(0){}
};

struct LocationData : public CommonConfig 
{
    std::set<std::string> methods;
};

struct Server : public CommonConfig 
{
    std::vector<std::pair<std::string, int> > listen;
    std::vector<std::string> server_names;
    std::map<std::string, LocationData> locations;
    bool isvirtual;
    std::vector<std::pair<std::string, int> > virtual_listen;
    Server():isvirtual(false){}
};


#endif