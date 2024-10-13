#include <tcpserver.h>
#include <errno.h>
#include <string.h>

TCPServer::TCPServer(std::function<void(int, std::string)> onError) : BaseSocket(onError, TCP)
{
    int opt = 1;
    setsockopt(this->sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(int));
    setsockopt(this->sock,SOL_SOCKET,SO_REUSEPORT,&opt,sizeof(int));
}

void TCPServer::Bind(int port, std::function<void(int, std::string)> onError)
{
    this->Bind("0.0.0.0", port, onError);
}

void TCPServer::Bind(const char *address, uint16_t port, std::function<void(int, std::string)> onError)
{
    if (inet_pton(AF_INET, address, &this->address.sin_addr) <= 0)
    {
        onError(errno, "Invalid address. Address type not supported.");
        exit(1);
    }

    this->address.sin_family = AF_INET;
    this->address.sin_port = htons(port);

    if (bind(this->sock, (const sockaddr *)&this->address, sizeof(this->address)) < 0)
    {
//	printf("Cannot bind the socket: %d - %s\n", errno, strerror(errno));
//	if (errno == 13) printf("please try using 'sudo' \n");
        onError(errno, "Cannot bind the socket. try using 'sudo'");
        exit(1);
    }
}

void TCPServer::Listen(std::function<void(int, std::string)> onError)
{
    if (listen(this->sock, 10) < 0)
    {
        onError(errno, "Error: Server can't listen the socket.");
        exit(1);
    }

    std::thread acceptThread(Accept, this, onError);
    acceptThread.detach();
}

void TCPServer::Close()
{
    shutdown(this->sock, SHUT_RDWR);
    
    BaseSocket::Close();
}

void TCPServer::Accept(TCPServer *server, std::function<void(int, std::string)> onError)
{
    sockaddr_in newSocketInfo;
    socklen_t newSocketInfoLength = sizeof(newSocketInfo);

    int newSock;
    while (!server->isClosed)
    {
        while ((newSock = accept(server->sock, (sockaddr *)&newSocketInfo, &newSocketInfoLength)) < 0)
        {
            if (errno == EBADF || errno == EINVAL) return;

            onError(errno, "Error while accepting a new connection.");
            return;
        }

        if (!server->isClosed && newSock >= 0)
        {
            TCPSocket *newSocket = new TCPSocket([](int e, std::string er) { FDR_UNUSED(e); FDR_UNUSED(er); }, newSock);
            newSocket->setAddressStruct(newSocketInfo);

            server->onNewConnection(newSocket);
            newSocket->Listen();
        }
    }
}
