/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:11:26 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/16 17:10:20 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Lexer.hpp"

Lexer::Lexer(std::ifstream &Conf): ConfigFile(Conf),
		pos(0)
{
	
}


void Lexer::skipWhitespace()
{
	while (pos < RdBuf.length() && isspace(RdBuf[pos]))
		pos++;
}

void Lexer::skipComment()
{
	while (pos < RdBuf.length() && RdBuf[pos] != '\n')
		pos++;
}

void Lexer::skipemptyLines()
{
	while (pos < RdBuf.length() && (RdBuf[pos] == '\n'))
		pos++;
}

void Lexer::ReadLine()
{

	pos = 0;
	getline(ConfigFile, RdBuf);
}
Token Lexer::getNextToken()
{
	this->skipWhitespace();
	if (RdBuf[pos] == '#')
		this->skipComment();
	if (RdBuf[pos] == '\n' || RdBuf[pos] == '\0' || pos >= RdBuf.length())
	{
		this->ReadLine();
		if (ConfigFile.eof())
			return Token(TOKEN_EOF, "");
		return getNextToken();
	}
	if (isalpha(RdBuf[pos]) || RdBuf[pos] == '/' || RdBuf[pos] == '_' || RdBuf[pos] == '.')
	{
		size_t start = pos;
		while (pos < RdBuf.length() && (isalnum(RdBuf[pos]) || RdBuf[pos] == '_' || RdBuf[pos] == '.' || RdBuf[pos] == '/'))
			pos++;
		std::string value = RdBuf.substr(start, pos - start);
		if (value.find('/') != std::string::npos)
			return Token(TOKEN_PATH, value );
		else
			return Token(TOKEN_IDENTIFIER, value );
	}
	if (isdigit(RdBuf[pos]))
	{
		size_t start = pos;
		while (pos < RdBuf.length() && (isdigit(RdBuf[pos]) || RdBuf[pos] == '.'))
			pos++;
		std::string value = RdBuf.substr(start, pos - start);
		if (value.find('.') != std::string::npos)
			return Token( TOKEN_IDENTIFIER, value );
		return Token( TOKEN_NUMBER, value );
	}

	switch  (RdBuf[pos])
	{
		case '{':
			pos++;
			return Token( TOKEN_OPEN_BRACE, "{" );
		case '}':
			pos++;
			return Token( TOKEN_CLOSE_BRACE, "}" );
		case ';':
			pos++;
			return Token( TOKEN_SEMICOLON, ";" );
		case ':':
			pos++;
			return Token( TOKEN_COLON, ":" );
	}
	return Token( TOKEN_EOF, "" );
}
