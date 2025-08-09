/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/23 15:08:59 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/09 19:45:25 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP
#include "Lexer.hpp"
#include <vector>
#include "ConfigData.hpp"
#include <exception>
#include <fstream>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <algorithm>
#include "Exceptions.hpp"

class Parser
{
	std::vector<Server> &servers;
	Lexer lexer;
	Token currentToken;
	std::string tmpkey;
	
public:
	Parser(std::ifstream &,std::vector<Server> &);
	void Config();
	void ServerBlock();
	void DirectiveList();
	void Directive();
	void ListenDirective();
	void Host();
	void Port();
	// void IpAddress();
	void ServerNameDirective();
	void ErrorPageDirective(std::map<int, std::string > &);
	void ErrorPageList(std::set<int>&);
	void ClientMaxBodySizeDirective(size_t &);
	void LocationBlock();
	void LocationDirectiveList();
	void LocationDirective();
	void RootDirective(std::string &);
	void AliasDirective(std::string &alias);
	void MethodsDirective();
	void ReturnDirective(std::pair<int, std::string> &, bool&);
	void AutoindexDirective(bool&);
	void IndexDirective(std::string &);
	void CgiDirective(std::map<std::string, std::string> &);
	void UploadDirective(bool&,std::string &);
	
};

#endif