/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:07:58 by eaboudi           #+#    #+#             */
/*   Updated: 2025/08/21 18:40:46 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef             CGI_HPP
#define             CGI_HPP

#include <netinet/in.h>
#include <unistd.h>
#include "Connection.hpp"
// #include <arpa/inet.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <fcntl.h>
#include <sys/wait.h>
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <exception>
#include <fstream>
#include <string>

#define CONTENT_TOO_LARGE 19

class Connection;

class CGI
{
    // private:
    public:
        std::string     SCRIPT_PATH;
        std::string     SCRIPT_NAME;
        std::string     QUERY_STRING;
        std::string     PATH_INFO;
        std::string     SERVER_PROTOCOL;
        std::string     SERVER_NAME;
        int             SERVER_PORT;
        std::string     REMOTE_ADDR;
        std::string     REMOTE_PORT;
        std::string     REMOTE_IDENT;
        std::string     CONTENT_TYPE; 
        std::string     CONTENT_LENGTH;
        std::string     REQUEST_METHOD;
        
        std::string     Ext;
        std::string     InFile; 
        std::string     OutFile;
        size_t          InSize;
        char            **Env;
        std::map<std::string, std::string> CgiHeaders;
        size_t          OutputSize;
        std::string     PostBodyFile;
        std::vector<std::string> EnvString;
        pid_t   Pid;
        bool    Is_Runing;

        // add time to check timeout
        
        
        CGI();
        ~CGI();
        void            ExecuteCgi(Connection *conn);
        void            BuildEnv(Connection *conn);
        bool            IsCgiComplet(Connection *conn);
};

#endif