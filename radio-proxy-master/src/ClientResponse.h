#ifndef RADIO_PROXY_CLIENTRESPONSE_H
#define RADIO_PROXY_CLIENTRESPONSE_H

#include "ICYResponse.h"
#include "UDPServer.h"

class ClientResponse : public ICYResponse{
private:
    int byte_count = 0;

    void meta_data_block() override;

    void parse_raw_response() override;

    void parse_response_meta() override;
public:
    ClientResponse(int tcp_fd, std::vector<std::string> &headers, po::variables_map &args) :
        ICYResponse(tcp_fd, headers, args) {}
    void send_response() override ;
};


#endif //RADIO_PROXY_CLIENTRESPONSE_H
