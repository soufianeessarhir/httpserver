/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:19:37 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/30 08:46:45 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "CGI.hpp"

CGI::CGI()
{
    
}

void    CGI::BuildEnv(Connection *conn)
{
    EnvString.push_back("SCRIPT_PATH=" + SCRIPT_PATH);
    EnvString.push_back("SCRIPT_NAME=" + SCRIPT_NAME);
    EnvString.push_back("QUERY_STRING=" + QUERY_STRING);
    EnvString.push_back("PATH_INFO=" + PATH_INFO );
    EnvString.push_back("SERVER_PROTOCOL=" + SERVER_PROTOCOL);
    EnvString.push_back("SERVER_NAME=" + SERVER_NAME);
    EnvString.push_back("SERVER_PORT=" + SERVER_PORT);
    EnvString.push_back("REMOTE_ADDR=" + REMOTE_ADDR);
    EnvString.push_back("REMOTE_PORT=" + REMOTE_PORT);
    EnvString.push_back("ROOT_FOLDER=" + conn->location->root);
    Env = new char*[EnvString.size() + 1];
    for (size_t i(0); i < EnvString.size(); i++)
        Env[i] = const_cast<char *>(EnvString[i].c_str());
    Env[EnvString.size()] = NULL;
}

void CGI::ExecuteCgi()
{
    std::cout << Ext << std::endl;
}

CGI::~CGI()
{
    
}