#include <udpserver.h>

UDPServer::UDPServer()
{
}

void UDPServer::Bind(int port, std::function<void(int, std::string)> onError) {
    this->Bind("0.0.0.0", port, onError);
}

void UDPServer::Bind(std::string IPv4, std::uint16_t port, std::function<void(int, std::string)> onError)
{
    if (inet_pton(AF_INET, IPv4.c_str(), &this->address.sin_addr) <= 0)
    {
        onError(errno, "Invalid address. Address type not supported.");
        return;
    }

    this->address.sin_family = AF_INET;
    this->address.sin_port = htons(port);

    if (bind(this->sock, (const sockaddr *)&this->address, sizeof(this->address)) < 0)
    {
        onError(errno, "Cannot bind the socket.");
        return;
    }
}

void UDPServer::BindMulticast(int port, const char * IPv4Mask, std::function<void(int, std::string)> onError) {
    this->BindMulticast("0.0.0.0", port, IPv4Mask, onError);
}
void UDPServer::BindMulticast(std::string IPv4, std::uint16_t port,const char * IPv4Mask, std::function<void(int, std::string)> onError)
{
    if (inet_pton(AF_INET, IPv4.c_str(), &this->address.sin_addr) <= 0)
    {
        onError(errno, "Invalid address. Address type not supported.");
        return;
    }

    this->address.sin_family = AF_INET;
    this->address.sin_port = htons(port);

	this->address.sin_addr.s_addr=htonl(INADDR_ANY);

    if (bind(this->sock, (const sockaddr *)&this->address, sizeof(this->address)) < 0)
    {
        onError(errno, "Cannot bind the socket.");
        return;
    }

	ip_mreqn group;

	group.imr_multiaddr.s_addr = inet_addr(IPv4Mask);
	group.imr_address.s_addr = htonl(INADDR_ANY);
	group.imr_ifindex = 0;

	if (setsockopt(this->sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) == -1) {
		perror("Error joining multicast group");
		exit(1);
	}

/*

out_addr.s_addr = inet_addr("192.168.1.1");   // your outgoing interface IP
if (setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, (char *)&out_addr,sizeof(out_addr))== -1) {
    perror("Error setting outgoing interface");
    exit(1);
}
*/
}
