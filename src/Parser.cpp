/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/23 15:10:27 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/10 19:36:25 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Parser.hpp"

Parser::Parser(std::ifstream &cfg, std::vector<Server> &servers)
:servers(servers),lexer(cfg),currentToken(TOKEN_EOF, "")
{

	
}

void		Parser::Config()
{
	currentToken = lexer.getNextToken();
	while (currentToken.type != TOKEN_EOF)
	{
		servers.push_back(Server());
		if (currentToken.value == "server")
			ServerBlock();
		else
			throw ParseException("Expected 'server' or 'EOF'");
		currentToken = lexer.getNextToken();
	}
	ConfigValidator conf(this->servers);
	conf.ValidateConfig();
}

void		Parser::ServerBlock()
{
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_OPEN_BRACE)
	throw ParseException("Expected '{' after 'server'");
	currentToken = lexer.getNextToken();
	DirectiveList();
	if (currentToken.type != TOKEN_CLOSE_BRACE)
		throw ParseException("Expected '}' at the end of server block");
}

void		Parser::DirectiveList()
{
	while (currentToken.type != TOKEN_CLOSE_BRACE && currentToken.type != TOKEN_EOF)
	{
		Directive();
		currentToken = lexer.getNextToken();
	}
}

void		Parser::Directive()
{
	if (currentToken.value == "listen")
		ListenDirective();
	else if (currentToken.value == "server_name")
		ServerNameDirective();
	else if (currentToken.value == "root")
		RootDirective(servers.back().root);
	else if (currentToken.value == "index")
		IndexDirective(servers.back().index);
	else if (currentToken.value == "return")
		ReturnDirective(servers.back().redirect, servers.back().has_redirect);
	else if(currentToken.value == "autoindex")
		AutoindexDirective(servers.back().autoindex);
	else if (currentToken.value == "cgi")
		CgiDirective(servers.back().cgi);
	else if (currentToken.value == "upload_store")
		UploadDirective(servers.back().upload, servers.back().upload_store);
	else if (currentToken.value == "error_page")
		ErrorPageDirective(servers.back().error_pages);
	else if (currentToken.value == "client_max_body_size")
		ClientMaxBodySizeDirective(servers.back().max_body_size);
	else if (currentToken.value == "location")
		LocationBlock();
	else
		throw ParseException("Unknown directive");
}

void		Parser::ListenDirective()
{
	servers.back().listen.push_back(std::pair<std::string, int>("", -1));
	currentToken = lexer.getNextToken();
	if (currentToken.type == TOKEN_IDENTIFIER)
	{
		Host();
		currentToken = lexer.getNextToken();
		if (currentToken.type == TOKEN_COLON)
		{
			currentToken = lexer.getNextToken();
			if (currentToken.type == TOKEN_NUMBER)
				Port();
			else
				throw ParseException("Expected port number after ':'");
		}
		else 
			throw ParseException("Expected ':' after host");
	}
	else if (currentToken.type == TOKEN_NUMBER)
	{
		if (currentToken.value.find('.') != std::string::npos)
		{
			Host();
			currentToken = lexer.getNextToken();
			if (currentToken.type == TOKEN_COLON)
			{
				currentToken = lexer.getNextToken();
				if (currentToken.type == TOKEN_NUMBER)
					Port();
				else
					throw ParseException("Expected port number after ':'");
			}
			else
				throw ParseException("Expected ':' or port number after IP address");
		}
		else
		{
			servers.back().listen.back().first = "0.0.0.0";
			Port();
		}
	}
	else
		throw ParseException("Expected host or port number after 'listen'");
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'listen' directive");
}

void		Parser::Host()
{
	if (currentToken.value == "localhost")
	{
		servers.back().listen.back().first = "127.0.0.1";
		return ;
	}
	std::string val;
	size_t pos = 0;
	size_t prevpos = 0;
	for (int i = 0; i < 4;++i)
	{
		pos = currentToken.value.find('.',pos);
		if (pos != std::string::npos)
		{
			val = currentToken.value.substr(prevpos,pos - prevpos);
			prevpos = ++pos;
		}
		else if(pos == std::string::npos && i == 3)
			val = currentToken.value.substr(prevpos);
		else 
			throw ParseException("Expected 'ipv4 address or localhost' in 'listen' directive");
		errno = 0;
		char *endptr;
		long long port = strtol(val.c_str(), &endptr, 10);
		if (errno == ERANGE || *endptr != '\0' || port < 0 || port > 255)
			throw ParseException("Expected 'ipv4 address or localhost' in 'listen' directive");
	}
	servers.back().listen.back().first = currentToken.value;
}

void		Parser::Port()
{
	errno = 0;
	char *endptr;
	long long port = strtol(currentToken.value.c_str(), &endptr, 10);
	if (errno == ERANGE || *endptr != '\0' || port < 0 || port > 65535)
		throw ParseException("Port number out of range");
	servers.back().listen.back().second = static_cast<int>(port);
}

void		Parser::ServerNameDirective()
{
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_IDENTIFIER)
		throw ParseException("Expected server name after 'server_name'");
	if (std::find(servers.back().server_names.begin(),servers.back().server_names.end(),currentToken.value) == servers.back().server_names.end())
	{
		servers.back().server_names.push_back(currentToken.value);
		currentToken = lexer.getNextToken();
		while (currentToken.type != TOKEN_SEMICOLON && currentToken.type != TOKEN_EOF)
		{
			if (currentToken.type == TOKEN_IDENTIFIER)
				servers.back().server_names.push_back(currentToken.value);
			else
				throw ParseException("Expected server name");
			currentToken = lexer.getNextToken();
		}
	}
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'server_name' directive");
}


void		Parser::ErrorPageDirective(std::map<int,std::string > & error_pages)
{
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_NUMBER)
		throw ParseException("Expected error code after 'error_page'");
	std::set<int> tmpset;
	ErrorPageList(tmpset);
	if (currentToken.type != TOKEN_PATH && currentToken.type != TOKEN_IDENTIFIER)
		throw ParseException("Expected path after error code");
	for (std::set<int>::iterator  it = tmpset.begin();it != tmpset.end();++it)
		error_pages[*it] =  currentToken.value;
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'error_page' directive");
}

void		Parser::ErrorPageList(std::set<int> & tmpset)
{
	while (currentToken.type == TOKEN_NUMBER)
	{
		char *endptr;
		errno = 0;
		long long errorCode = strtol(currentToken.value.c_str(), &endptr, 10);
		if (errno == ERANGE || *endptr != '\0' || errorCode < 100 || errorCode > 599)
			throw ParseException("Error code out of range");
		tmpset.insert(static_cast<int>(errorCode));
		currentToken = lexer.getNextToken();
	}
}

void		Parser::ClientMaxBodySizeDirective(size_t & max_body_size)
{
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_NUMBER)
		throw ParseException("Expected size after 'client_max_body_size'");
    errno = 0;
	char *endptr;
	long long size = strtol(currentToken.value.c_str(), &endptr, 10);
	if (errno == ERANGE || *endptr != '\0' || size < 0)
		throw ParseException("Size out of range");
	currentToken = lexer.getNextToken();
	max_body_size = static_cast<size_t>(size);
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'client_max_body_size' directive");
}

void		Parser::LocationBlock()
{
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_PATH)
	throw ParseException("Expected location path after 'location'");
	tmpkey = currentToken.value;
	servers.back().locations[tmpkey] = LocationData();
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_OPEN_BRACE)
		throw ParseException("Expected '{' after location path");
	LocationDirectiveList();
	if (currentToken.type != TOKEN_CLOSE_BRACE)
		throw ParseException("Expected '}' at the end of location block");
}

void		Parser::LocationDirectiveList()
{
	currentToken = lexer.getNextToken();
	while (currentToken.type != TOKEN_CLOSE_BRACE && currentToken.type != TOKEN_EOF)
	{
		LocationDirective();
		currentToken = lexer.getNextToken();
	}
}

void		Parser::LocationDirective()
{
	if (currentToken.value == "root")
		RootDirective(servers.back().locations[tmpkey].root);
	else if (currentToken.value == "index")
		IndexDirective(servers.back().locations[tmpkey].index);
	else if (currentToken.value == "return")
		ReturnDirective(servers.back().locations[tmpkey].redirect, servers.back().locations[tmpkey].has_redirect);
	else if (currentToken.value == "autoindex")
		AutoindexDirective(servers.back().locations[tmpkey].autoindex);
	else if (currentToken.value == "cgi")
		CgiDirective(servers.back().locations[tmpkey].cgi);
	else if (currentToken.value == "upload_store")
		UploadDirective(servers.back().locations[tmpkey].upload, servers.back().locations[tmpkey].upload_store);
	else if (currentToken.value == "error_page")
		ErrorPageDirective(servers.back().locations[tmpkey].error_pages);
	else if (currentToken.value == "client_max_body_size")
		ClientMaxBodySizeDirective(servers.back().locations[tmpkey].max_body_size);
	else if (currentToken.value == "methods")
		MethodsDirective();
	else
		throw ParseException("Unknown location directive");
}

void		Parser::RootDirective(std::string &root)
{
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_PATH)
		throw ParseException("Expected path after 'root'");
    root = currentToken.value;
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'root' directive");
}

void		Parser::MethodsDirective()
{
	currentToken = lexer.getNextToken();
	while (currentToken.value == "GET" || currentToken.value == "POST" || currentToken.value == "DELETE")
	{
		if (std::find(servers.back().locations[tmpkey].methods.begin(), servers.back().locations[tmpkey].methods.end(), currentToken.value) == servers.back().locations[tmpkey].methods.end())
			servers.back().locations[tmpkey].methods.insert(currentToken.value);
		else
			throw ParseException("Duplicate method in 'methods' directive");
		currentToken = lexer.getNextToken();
	}
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'methods' directive");	
}

void		Parser::ReturnDirective(std::pair<int, std::string> &redirect , bool& has_redirect)
{
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_NUMBER)
		throw ParseException("Expected status code after 'return'");
	errno = 0;
	char *endptr;
	long long statusCode = strtol(currentToken.value.c_str(), &endptr, 10);
	if (errno == ERANGE || *endptr != '\0' || statusCode < 100 || statusCode > 599)
		throw ParseException("Status code out of range");
	has_redirect = true;
	redirect.first = static_cast<int>(statusCode);
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_PATH)
		throw ParseException("Expected path after status code");
	redirect.second = currentToken.value;
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'return' directive");
}

void		Parser::AutoindexDirective(bool &autoindex)
{
	currentToken = lexer.getNextToken();
	if (currentToken.value != "on" && currentToken.value != "off")
		throw ParseException("Expected 'on' or 'off' after 'autoindex'");
	autoindex = (currentToken.value == "on");
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'autoindex' directive");
}

void		Parser::IndexDirective(std::string &index)
{
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_IDENTIFIER)
		throw ParseException("Expected path after 'index'");
	currentToken = lexer.getNextToken();
	while (currentToken.type == TOKEN_IDENTIFIER)
	{
		index = currentToken.value;
		currentToken = lexer.getNextToken();
	}
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'index' directive");
}


void		Parser::CgiDirective( std::map<std::string, std::string> &cgi)
{
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_IDENTIFIER)
		throw ParseException("Expected IDENTIFIER after cgi directive");
	if (currentToken.value[0] != '.')
		throw ParseException("Expected '.' in  'cgi' extention");
	std::string extension = currentToken.value;
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_PATH)
		throw ParseException("Expected path after 'cgi' path");
	cgi[extension] = currentToken.value;
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'cgi' directive");
}

void		Parser::UploadDirective(bool &upload,std::string &upload_store)
{
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_PATH && currentToken.type != TOKEN_IDENTIFIER)
		throw ParseException("Expected path after 'upload_store'");
	upload = true;
	upload_store = currentToken.value;
	currentToken = lexer.getNextToken();
	if (currentToken.type != TOKEN_SEMICOLON)
		throw ParseException("Expected ';' after 'upload_store' directive");
}


