#include "Request.hpp"
#include "MainResponse.hpp"

void    ExecuteGET(Connection *conn)
{
    if (!conn->response->GET)
        conn->response->GET = new MainResponse(conn->response->GetStatusCode());
    switch (conn->response->GET->ResponseStat)
    {   
        case SENDING_STATUSLINE :
        {
            if (conn->response->GET->CheckForSending(conn) == false)
                return ;
            conn->response->GET->SetContentType(conn);
            conn->response->GET->SetStatusLine();
            conn->response->GET->SendStatusLine(conn);
            conn->response->GET->SetHeaders(false, conn);
            conn->response->GET->SendHeaders(conn);
            conn->response->GET->ResponseStat = SENDING_BODY;
            break;
        }
        case SENDING_BODY :
        {
            conn->response->GET->SetAndSendBody(conn);
            if (conn->response->GET->ResponseStat == SENDING_COMPLETE)
            {    
                conn->state = Connection::COMPLETE;
                delete conn->response->GET;
                conn->response->GET = NULL;
            }
            break;
        }
        case SENDING_COMPLETE :
            break ;
    }
}