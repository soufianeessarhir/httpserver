#include "Response.hpp"
#include "MainResponse.hpp"
#include "Post.hpp"

void PostResponse(Connection *conn)
{
    if (!conn->response->POST)
    {
        conn->response->POST = new MainResponse(conn->response->GetStatusCode());
        std::string Path;
        Path = conn->response->POST->ErrorHtmlPath.find(conn->response->POST->GetStatusCode())->second;
        conn->request->SetUri(Path);
    }
    switch (conn->response->POST->ResponseStat)
    {   
        case SENDING_STATUSLINE :
        {
            conn->response->POST->CheckForSending(conn);
            conn->response->POST->SetContentType(conn);
            conn->response->POST->SetStatusLine();
            conn->response->POST->SendStatusLine(conn);
            conn->response->POST->SetHeaders(false, conn);
            conn->response->POST->SendHeaders(conn);
            conn->response->POST->ResponseStat = SENDING_BODY;
            break;
        }
        case SENDING_BODY :
        {
            conn->response->POST->SetAndSendBody(conn);
            if (conn->response->POST->ResponseStat == SENDING_COMPLETE)
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