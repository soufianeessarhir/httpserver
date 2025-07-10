#include <string>
#include <iostream>
#include <fstream>
// #include "include/Connection.hpp"
#include <unistd.h>
#include <signal.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>

// static std::vector<std::string> EnvString;

// char**    BuildEnv()
// {
//     char **Env;
//     std::string REQUEST_METHOD  = "GET";
//     std::string QUERY_STRING = "Name=eaboudi";
//     std::string REMOTE_ADDR = "198.124.0.0";
//     std::string SERVER_PROTOCOL = "HTTP/1.1";
//     // std::vector<std::string> EnvString;

//     EnvString.clear();
//     EnvString.push_back("REQUEST_METHOD=" + REQUEST_METHOD);
//     EnvString.push_back("QUERY_STRING=" + QUERY_STRING);
//     EnvString.push_back("REMOTE_ADDR=" + REMOTE_ADDR);
//     EnvString.push_back("SERVER_PROTOCOL=" + SERVER_PROTOCOL);
//     Env = new char*[EnvString.size() + 1];
//     for (int i(0); i < EnvString.size(); i++)
//         Env[i] = const_cast<char *>(EnvString[i].c_str());
//     Env[EnvString.size()] = NULL;
//     return Env;
// }

// void    ExecuteCgi()
// {
//     int Pipe[2];
//     std::string FileName = "bin/tmp";
//     std::fstream TmpFile(FileName, std::ios::in | std::ios::out);
//     char **Env = BuildEnv();
//     pipe(Pipe);
//     int Child = fork();
//     if (Child == -1)
//     {
//         perror("fork");
//         return;
//     }
//     if (Child == 0)
//     {
//         close(Pipe[0]);
//         int tmpfd = open(FileName.c_str(), O_CREAT | O_RDWR | O_TRUNC, 0666);
//         if (tmpfd == -1)
//             perror("open");
//         dup2(tmpfd, STDOUT_FILENO);
//         close(tmpfd);
//         char *argv[] = { (char*)"/home/eaboudi/Desktop/httpserver/bin/cgi.sh", NULL};
//         if (execve("/home/eaboudi/Desktop/httpserver/bin/cgi.sh", argv, Env) == -1)
//         {
//             perror("execve");
//             return ;
//         }
//     }
//     else
//     {
//         close(Pipe[1]);
//         char buff[4096];
//         ssize_t byteread;
//         std::string output;
//         while((byteread = read(Pipe[0], buff, sizeof(buff))) > 0)
//         {
//             output.append(buff, byteread);
//         }
//         close(Pipe[0]);
//         int status;
//         waitpid(Child, &status, 0);
//         delete[] Env;
//     }
// }


// void    CheckCgiExist(std::string Uri)
// {
//     size_t Pos = Uri.find('?');
//     std::string Path, Script;
//     if (Pos != Uri.npos)
//         Path = Uri.substr(0, Pos);
//     else
//         Path = Uri;
//     size_t scriptPos = Path.find_last_of('/');
//     Script = Path.substr(scriptPos + 1);
// }
    // bool    ShebangDetection(std::string Path) //add by eaboudi
    // {
        
    //     if (stat(Path.c_str(), &FileState) == 0)
    //     {
    //         if (S_ISREG(FileState.st_mode) && (FileState.st_mode & S_IXUSR) && (FileState.st_mode & S_IREAD))
    //         { 
    //             std::fstream File(Path, std::ios::in);
    //             std::string Line;
    //             while (std::getline(File, Line))
    //             {
    //                 for (int i(0); i < Line.size(); i++)
    //                 {
    //                     while(Line[i] && (Line[i] == ' ' || Line[i] == '\t'))
    //                         i++;
    //                     if (Line[i] && Line[i] == '#' && Line[i + 1] == '!')
    //                         return true;
    //                 }
    //             }
    //         }
    //     }
    //     return false;
    // }

bool    CheckMagicNumber(std::string Path)
{
    std::ifstream file(Path, std::ios::in | std::ios::binary);
    if (!file.is_open())
    {
        std::cout << "file ait't open" << std::endl;
        return false;
    }
    unsigned char magic[4] = {0};
    file.read(reinterpret_cast<char*>(magic), sizeof(magic));
    if (!file)
    {
        std::cout << "file ain't read" << std::endl;
        return false;
    }
    if (magic[0] == 0x7F && magic[1] == 'E' && magic[2] == 'L' && magic[3] == 'F')
        return true;
    return false;
}

void CheckCgiExist() // add by eaboudi
{
    std::string Path = "/cgi-bin/foo/bar/?name=John";
    std::string QueryString;
    size_t Pos = Path.find('?');
    if (Pos != std::string::npos)
    {
        QueryString = Path.substr(Pos + 1);
        Path = Path.substr(0, Pos);
    }
    Pos = Path.find('.');
    if (Pos != Path.npos)
    {
        std::string PathInfo;
        if (Path.find('/', Pos) != Path.npos)
        {
            while(Path[Pos] != '/')
                Pos++;
            PathInfo = Path.substr(Pos);
        }
        else
            Pos = Path.size() - 1;
        std::string CheckDir = Path.substr(0, Pos);
        Pos = CheckDir.find_last_of('/');
        std::string ScriptName = CheckDir.substr(Pos + 1);
        CheckDir = CheckDir.substr(0, Pos);
        std::string CgiDir = "/cgi-bin";
        if (CheckDir.compare(0, CgiDir.size(), CgiDir) == 0)
        {
            CheckDir += '/';
            CheckDir += ScriptName;
            struct stat FileState;
            if (stat(CheckDir.c_str() + 1, &FileState) == 0)
            {
                if (S_ISREG(FileState.st_mode))
                    std::cout << "now you can execute" << std::endl;
            }
        }
    }
    std::cout << Path << std::endl;
}

int main()
{
    // ExecuteCgi();/home/eaboudi/Desktop/httpserver/bin/cgi.
    
    CheckCgiExist();
}

