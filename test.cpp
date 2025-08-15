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
#include <string>


int main()
{
        char *Env[] = {"hoem", "test", NULL};
        const char * argv[] = {"CGI_SCRIPTS/php_cgimac", "/Users/eaboudi/Desktop/httpserver/CGI_SCRIPTS/Login.php", NULL};
        if (execve(argv[0], const_cast<char **>(argv), Env) == -1)
        {
            perror("execve: ");
            exit(EXIT_FAILURE);
        }
}