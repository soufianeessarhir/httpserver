#include "Request.hpp"
#include "MainResponse.hpp"

void    ExecuteGET(Connection *conn)
{
    if (!conn->response->GET)
        conn->response->GET = new MainResponse(conn->response->GetStatusCode(), conn->request->GetUri()); //protect
    switch (conn->response->GET->ResponseStat)
    {   
        case SENDING_STATUSLINE :
        {
            conn->response->GET->CheckForSending(conn);
            conn->response->GET->SetContentType();
            conn->response->GET->SetStatusLine();
            conn->response->GET->SendStatusLine(conn);
            conn->response->GET->SetHeaders(false, conn->request);
            conn->response->GET->SendHeaders(conn);
            conn->response->GET->ResponseStat = SENDING_BODY;
            break;
        }
        case SENDING_BODY :
        {
            conn->response->GET->SetAndSendBody(conn);
            // If file sending is complete, update state
            if (conn->response->GET->ResponseStat == SENDING_COMPLETE)
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