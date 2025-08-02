/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/23 15:08:59 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/02 19:43:25 by sessarhi         ###   ########.fr       */
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
	void IpAddress();
	void ServerNameDirective();
	void ErrorPageDirective(std::vector<std::pair<std::vector<int>, std::string> > &);
	void ErrorPageList(std::vector<std::pair<std::vector<int>, std::string> > &);
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

class ParseException : public std::exception
{	
public:
	ParseException(const std::string &message) : msg_(message) {}
	virtual const char* what() const throw() { return msg_.c_str(); }
	virtual ~ParseException() throw() {}
private:
	std::string msg_;
};
#endif