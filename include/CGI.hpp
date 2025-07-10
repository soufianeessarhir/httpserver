/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CGI.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: eaboudi <eaboudi@student.1337.ma>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/06 10:07:58 by eaboudi           #+#    #+#             */
/*   Updated: 2025/07/10 11:23:02 by eaboudi          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef             CGI_HPP
#define             CGI_HPP

#include <netinet/in.h>
// #include "Connection.hpp"
#include <arpa/inet.h>
#include <sys/stat.h>
#include <string>
#include <vector>
#include <map>
#include <set>

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
        std::string     SCRIPT_PATH;
        std::string     SCRIPT_NAME;
        std::string     QUERY_STRING;
        std::string     PATH_INFO;
        std::string     REQUEST_METHOD;
        std::string     CONTENT_TYPE;
        ssize_t         CONTENT_LENGTH;
        std::string     BODY;
        std::string     SERVER_PROTOCOL;
        std::string     SERVER_NAME;
        std::string     SERVER_PORT;
        std::string     REMOTE_ADDR;
        int             REMOTE_PORT;
        char            **Env;
        t_ExecuteCgiParams Vars;
        // add time to check timeout
        
        
        CGI();
        ~CGI();
        void            ParseQueryString();
        void            SplitQueryPrams();
        void            DecodeUri();
        void            ExecuteCgi();
        void            BuildEnv();
};

#endif