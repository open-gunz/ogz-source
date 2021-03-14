#ifndef RADIO_PROXY_UDPCLIENT_H
#define RADIO_PROXY_UDPCLIENT_H


#include <netinet/in.h>
#include <chrono>

class UDPClient {
private:
    struct sockaddr_in address{};
    time_t last_request{};
    bool banned;
public:
    explicit UDPClient(sockaddr_in);
    void set_last_request(time_t);
    bool is_equal(const UDPClient&) const;
    bool is_banned() const {
        return banned;
    }
    time_t get_last_request() const {
        return last_request;
    }
    void ban() {
        banned = true;
    }
    sockaddr_in get_addr() {
        return address;
    }
};


#endif //RADIO_PROXY_UDPCLIENT_H
