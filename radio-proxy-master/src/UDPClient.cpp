#include "UDPClient.h"

UDPClient::UDPClient(sockaddr_in addr) {
    address = addr;
    banned = false;
}

void UDPClient::set_last_request(time_t time) {
    last_request = time;
}

bool UDPClient::is_equal(const UDPClient &udpClient) const {
    return this->address.sin_port == udpClient.address.sin_port &&
            this->address.sin_addr.s_addr == udpClient.address.sin_addr.s_addr &&
            this->address.sin_family == udpClient.address.sin_family;
}

