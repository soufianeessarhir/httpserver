#include "Response.hpp"
#include "MainResponse.hpp"

void    ExecuteError(Connection *conn)
{
    if (!conn->response->Error)
    {  
        conn->response->Error = new MainResponse(conn->response->GetStatusCode(), conn->request->GetUri());
        std::string Path = conn->response->Error->ErrorHtmlPath.find(conn->response->GetStatusCode())->second;
        conn->request->SetUri(Path);
    }
    
    switch (conn->response->Error->ResponseStat)
    {   
        case SENDING_STATUSLINE :
        {
            conn->response->Error->CheckForSending(conn);
            conn->response->Error->SetContentType();
            conn->response->Error->SetStatusLine();
            conn->response->Error->SendStatusLine(conn);
            conn->response->Error->SetHeaders(true, conn->request);
            conn->response->Error->SendHeaders(conn);
            conn->response->Error->ResponseStat = SENDING_BODY;
            break;
        }
        case SENDING_BODY :
        {
            conn->response->Error->SetAndSendBody(conn);
            if (conn->response->Error->ResponseStat == SENDING_COMPLETE)
                conn->state = Connection::COMPLETE;
            break;
        }
        case SENDING_COMPLETE :
        {
            conn->state = Connection::COMPLETE;
            break ;
        }
    }
}