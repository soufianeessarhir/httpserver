#include "Response.hpp"
#include "MainResponse.hpp"

void    ExecuteDelete(Connection *conn)
{
    
    if (!conn->response->DELETE)
    {    
        struct stat FileState;
        if (stat(conn->request->GetUri().c_str(), &FileState) == -1)
        {
            conn->response->SetStatusCode(404);
            conn->response->SetMethod(Error);
            return ;
        }
        if (!S_ISREG(FileState.st_mode))
        {
            conn->response->SetStatusCode(409);
            conn->response->SetMethod(Error);
            return ;
        }
    
        if (unlink(conn->request->GetUri().c_str()) == 0) 
        {
            conn->response->SetStatusCode(204);
            if (!conn->response->DELETE)
            {
                conn->response->DELETE = new MainResponse(conn->response->GetStatusCode());
                conn->request->SetUri(conn->response->DELETE->ErrorHtmlPath.find(204)->second);
            }
        } 
        else
        {
            conn->response->SetStatusCode(500);
            conn->response->SetMethod(Error);
            return ;
        }
    }
    
    switch (conn->response->DELETE->ResponseStat)
    {   
        case SENDING_STATUSLINE :
        {
            conn->response->DELETE->CheckForSending(conn);
            conn->response->DELETE->SetContentType(conn);
            conn->response->DELETE->SetStatusLine();
            conn->response->DELETE->SendStatusLine(conn);
            conn->response->DELETE->SetHeaders(false, conn->request);
            conn->response->DELETE->SendHeaders(conn);
            conn->response->DELETE->ResponseStat = SENDING_BODY;
            break;
        }
        case SENDING_BODY :
        {
            conn->response->DELETE->SetAndSendBody(conn);
            if (conn->response->DELETE->ResponseStat == SENDING_COMPLETE)
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
