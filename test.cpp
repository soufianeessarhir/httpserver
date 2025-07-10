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
    int Pipe[2];
    std::string FileName = "CGI-SCRIPTS/test";
    std::fstream TmpFile(FileName, std::ios::in | std::ios::out);
    char **Env = BuildEnv();
    // pipe(Pipe);
    int Child = fork();
    if (Child == -1)
    {
        perror("fork");
        return;
    }
    int fd = open(FileName.c_str(), O_CREAT | O_RDWR, 0666);
    if (Child == 0)
    {
        // close(Pipe[0]);
        // close
        dup2(fd, STDOUT_FILENO);
        close(fd);
        char *argv[] = { (char*)"CGI-SCRIPTS/cgi.sh", NULL};
        if (access("CGI-SCRIPTS/cgi.sh", X_OK) == -1)
            perror("access");
        if (execve("CGI-SCRIPTS/cgi.sh", argv, Env) == -1)
        {
            perror("execve");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // close(Pipe[1]);

        char buff[4096];
        ssize_t byteread;
        std::string output;
        while((byteread = read(fd, buff, sizeof(buff))) > 0)
        {
            output.append(buff, byteread);
        }
        // close(Pipe[0]);
        close(fd);
        int status;
        waitpid(Child, &status, 0);
        delete[] Env;
        std::cout << output << std::endl;
    }
}



int main()
{
    ExecuteCgi();
}

