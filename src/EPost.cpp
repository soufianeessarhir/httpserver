#include "Response.hpp"
#include "MainResponse.hpp"
#include "Post.hpp"

void PostResponse(Connection *conn)
{
    if (!conn->response->POST)
    {
        conn->response->POST = new MainResponse(conn->response->GetStatusCode());
        std::string Path;
        if (conn->CgiObj)
            Path = conn->CgiObj->OutFile;
        else
            Path = conn->response->POST->ErrorHtmlPath.find(conn->response->POST->GetStatusCode())->second;
        conn->request->SetUri(Path);
        if (conn->location && conn->location->has_redirect)
        {
            conn->response->POST->SendStatusLine(conn);
            conn->response->POST->ResponseStat = SENDING_COMPLETE;
            conn->state = Connection::COMPLETE;
            return ;
        }
    }
    switch (conn->response->POST->ResponseStat)
    {   
        case SENDING_STATUSLINE :
        {

            if (conn->response->POST->CheckForSending(conn) == false)
                return ;
            if (!conn->CgiObj)
                conn->response->POST->SetContentType(conn);
            conn->response->POST->SetStatusLine();
            conn->response->POST->SendStatusLine(conn);
            if (conn->CgiObj)
            {
                std::string location = conn->CgiObj->CgiHeaders["Location"];
                if (!location.empty())
                {    
                    conn->response->POST->ResponseStat = SENDING_BODY;
                    break ;
                }
            }
            conn->response->POST->SetHeaders(false, conn);
            conn->response->POST->SendHeaders(conn);
            conn->response->POST->ResponseStat = SENDING_BODY;
            break;
        }
        case SENDING_BODY :
        {
            conn->response->POST->SetAndSendBody(conn);
            if (conn->response->POST->ResponseStat == SENDING_COMPLETE)
            {        
                conn->state = Connection::COMPLETE;
                delete conn->response->POST;
                conn->response->POST = NULL;
                if (conn->CgiObj)
                    conn->CgiObj->Pid = -42;
            }
            break;
        }
        case SENDING_COMPLETE :
            break ;
    }
}