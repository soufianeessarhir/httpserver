/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:07:58 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/26 19:03:07 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef             CGI_HPP
#define             CGI_HPP

#include <netinet/in.h>
#include <unistd.h>
// #include "Connection.hpp"
#include <arpa/inet.h>
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

// class Connection;

typedef struct  s_ExecuteCgiParams
{
    int Child;
    int Pipe[2];
    int TmpFd;
}t_ExecuteCgiParams;
class CGI
{
    // private:
    public:
        std::string     SCRIPT_PATH; //done
        std::string     SCRIPT_NAME; //done
        std::string     QUERY_STRING; //done
        std::string     PATH_INFO; //done
        std::string     REQUEST_METHOD;//done
        std::string     CONTENT_TYPE; //done for GET
        ssize_t         CONTENT_LENGTH; // done for GET
        std::string     SERVER_PROTOCOL; //done
        std::string     SERVER_NAME;//done
        std::string     SERVER_PORT; //done
        std::string     REMOTE_ADDR; //done
        int             REMOTE_PORT; //done
        char            **Env;
        int             SCRIPT_FDO;
        std::string     Out_File;
        std::string     PostBodyFile;
        std::vector<std::string> EnvString;
        t_ExecuteCgiParams Vars;

        // add time to check timeout
        
        
        CGI();
        ~CGI();
        void            ExecuteCgi();
        void            BuildEnv();
};

#endif