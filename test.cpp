#include <string>
#include <iostream>
#include <fstream>
// #include "include/Connection.hpp"
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>

static std::vector<std::string> EnvString;

char**    BuildEnv()
{
    char **Env;
    std::string REQUEST_METHOD  = "GET";
    std::string QUERY_STRING = "Name=eaboudi";
    std::string REMOTE_ADDR = "198.124.0.0";
    std::string SERVER_PROTOCOL = "HTTP/1.1";
    // std::vector<std::string> EnvString;

    EnvString.clear();
    EnvString.push_back("REQUEST_METHOD=" + REQUEST_METHOD);
    EnvString.push_back("QUERY_STRING=" + QUERY_STRING);
    EnvString.push_back("REMOTE_ADDR=" + REMOTE_ADDR);
    EnvString.push_back("SERVER_PROTOCOL=" + SERVER_PROTOCOL);
    Env = new char*[EnvString.size() + 1];
    for (int i(0); i < EnvString.size(); i++)
        Env[i] = const_cast<char *>(EnvString[i].c_str());
    Env[EnvString.size()] = NULL;
    return Env;
}

void    ExecuteCgi()
{
    std::string FileName = "CGI-SCRIPTS/test";
    int fd = open(FileName.c_str(), O_CREAT | O_WRONLY | O_TRUNC, 0666);
    char **Env = BuildEnv();
    
    int Child = fork();
    if (Child == -1)
    {
        perror("fork");
        close(fd);
        return;
    }
    
    if (Child == 0)
    {
        // Child process: redirect stdout to file and execute CGI
        dup2(fd, STDOUT_FILENO);
        close(fd);
        
        char *argv[] = {(char*)"CGI-SCRIPTS/cgi.sh", NULL};
        if (execv("CGI-SCRIPTS/cgi.sh", argv) == -1)
        {
            perror("execv");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // Parent process: close write fd and wait for child
        close(fd);
        
        int status;
        waitpid(Child, &status, 0);
        
        // Now read the output from the file
        int read_fd = open(FileName.c_str(), O_RDONLY);
        if (read_fd != -1)
        {
            char buff[4096];
            ssize_t byteread;
            std::string output;
            
            while((byteread = read(read_fd, buff, sizeof(buff))) > 0)
            {
                output.append(buff, byteread);
            }
            close(read_fd);
            std::cout << output << std::endl;
        }
        
        delete[] Env;
    }
}



int main()
{
    ExecuteCgi();
}

