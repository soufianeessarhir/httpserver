/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Exceptions.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/07 11:03:53 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/07 11:07:41 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP
#include <exception>
#include <iostream>

class ParseException : public std::exception
{
    
public:
	ParseException(const std::string &message) : msg_(message) {}
	virtual const char* what() const throw() { return msg_.c_str(); }
	virtual ~ParseException() throw() {}

private:
	std::string msg_;
};

class HttpServerError : public std::exception
{
	public:
	
		HttpServerError(const char *msg) : message(msg) {}
		
		virtual const char* what() const throw()
		{
			return message;
		}
		
		virtual ~HttpServerError() throw() {}
		
	private:

		const char *message;
};

class HttpClientError : public std::exception
{
	public:
		const int client_fd;
		HttpClientError(const char *msg,const int fd) : client_fd(fd),message(msg){}
		
		virtual const char* what() const throw()
		{
			return message;
		}
		
		virtual ~HttpClientError() throw() {}
		
	private:

		const char *message;
};

#endif