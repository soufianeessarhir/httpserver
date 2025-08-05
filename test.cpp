void    CheckCgiExist(std::string path) // add by eaboudi
{
    std::string Path = conn->location->root + conn->request->GetUri();
    std::string QueryString;
    std::string CheckDir;
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
        std::string Ext;
        if (Path.find('/', Pos) != Path.npos)
        {
            while(Path[Pos] != '/')
                Pos++;
            PathInfo = Path.substr(Pos);
        }
        else
            Pos = Path.size();
        CheckDir = Path.substr(0, Pos);
        Pos = CheckDir.find_last_of('/');
        std::string ScriptName = CheckDir.substr(Pos + 1);
        CheckDir = CheckDir.substr(0, Pos);
        std::string CgiDir = CGiDir;
        if (CheckDir.compare(0, CgiDir.size(), CgiDir) == 0)
        {
            CheckDir += '/';
            std::string ScriptPath = CheckDir;
            CheckDir += ScriptName;
            struct stat FileState;
            if (stat(CheckDir.c_str(), &FileState) == 0)
            {
                if (S_ISREG(FileState.st_mode))
                {
                    conn->UseCgi = true;
                    conn->CgiObj = new CGI;
                    conn->CgiObj->QUERY_STRING = QueryString;
                    conn->CgiObj->REQUEST_METHOD = conn->request->GetMethod();
                    conn->CgiObj->SCRIPT_PATH = ScriptPath;
                    conn->CgiObj->SCRIPT_NAME = ScriptPath + ScriptName;
                    conn->CgiObj->PATH_INFO = PathInfo;
                    conn->CgiObj->REMOTE_ADDR = conn->ip;
                    conn->CgiObj->REMOTE_PORT = conn->port;
                    conn->CgiObj->SERVER_PROTOCOL = conn->request->GetVersion();
                    // if (conn->CgiObj->REQUEST_METHOD == "POST")
                    // {
                    //     conn->CgiObj->CONTENT_LENGTH  = conn->request->GetContentLenght();
                    //     conn->CgiObj->CONTENT_TYPE = conn->request->GetHeader("content_type");
                    // }
                    // else
                    // {
                        conn->CgiObj->CONTENT_LENGTH = 0;
                        conn->CgiObj->CONTENT_TYPE = "";
                    // }
                    return ;
                }
            }
        }
    }
    conn->request->SetUri(Path);
    conn->UseCgi = false;
}