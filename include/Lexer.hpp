/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/21 16:12:00 by sessarhi          #+#    #+#             */
/*   Updated: 2025/07/16 17:08:19 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
# define LEXER_HPP

#include <iostream>
#include <string>
#include <fstream>
#include <cctype>

enum TokenType 
{
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_PATH,
    TOKEN_OPEN_BRACE,
    TOKEN_CLOSE_BRACE,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_DOT,
    TOKEN_EOF
};
struct Token
{
    TokenType type;
    std::string value;
    Token(TokenType t, const std::string& v) : type(t), value(v) {}
};

class Lexer
{
private:
	std::string RdBuf;
	std::ifstream &ConfigFile;
	size_t pos;
public:
	Lexer(std::ifstream& Conf);
	void skipWhitespace();
	void skipComment();
	void skipemptyLines();
	void ReadLine();
	Token getNextToken();
};


#endif