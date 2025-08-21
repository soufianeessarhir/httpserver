#include "Response.hpp"
#include "MainResponse.hpp"

void    ExecuteError(Connection *conn)
{
    if (!conn->response->Error)
    {  
        std::string Path;
        conn->response->Error = new MainResponse(conn->response->GetStatusCode());
        if (conn->location->error_pages.find(conn->response->GetStatusCode()) != conn->location->error_pages.end())
            Path = conn->location->root + conn->location->error_pages[conn->response->GetStatusCode()];
        else
            Path = conn->response->Error->ErrorHtmlPath.find(conn->response->GetStatusCode())->second;
        conn->request->SetUri(Path);
        if (conn->CgiObj)
        {
            delete conn->CgiObj;
            conn->CgiObj = NULL;
            conn->UseCgi = false;
        }
    }
    switch (conn->response->Error->ResponseStat)
    {   
        case SENDING_STATUSLINE :
        {
            if (conn->response->Error->CheckForSending(conn) == false)
            {    
                ExecuteError(conn);
                return ;
            }    
            conn->response->Error->SetContentType(conn);
            conn->response->Error->SetStatusLine();
            conn->response->Error->SendStatusLine(conn);
            conn->response->Error->SetHeaders(true, conn);
            conn->response->Error->SendHeaders(conn);
            conn->response->Error->ResponseStat = SENDING_BODY;
            break;
        }
        case SENDING_BODY :
        {
            conn->response->Error->SetAndSendBody(conn);
            if (conn->response->Error->ResponseStat == SENDING_COMPLETE)
            {    
                conn->state = Connection::COMPLETE;
                delete conn->response->Error;
                conn->response->Error = NULL;
                
            }
            break;
        }
        case SENDING_COMPLETE :
            break ;
    }
}