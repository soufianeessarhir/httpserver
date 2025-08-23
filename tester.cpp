#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
int main() 
{

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
	signal(SIGPIPE,SIG_IGN);
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = 8000;
	addr.sin_addr = inet_pton(AF_INET,"127.0.0.1",);
  	connect(sock, (sockaddr*)&addr, sizeof(addr));
    std::string req = "GET /  HTTP/1.1";
	std::cout << "reach here\n";
    ssize_t sent = send(sock, req.c_str(), req.length(), 0);

    // Now, keep sending more and more on the request line without CRLF
    while (true) 
	{
        std::string chunk(1024, 'A'); // 1KB at a time
        sent = send(sock, chunk.c_str(), chunk.size(), 0);
		if (sent == 0)
			return 1;
    }
    return 0;
}