#ifndef RADIO_PROXY_DIRECTRESPONSE_H
#define RADIO_PROXY_DIRECTRESPONSE_H

#include "ICYResponse.h"

class DirectResponse : public ICYResponse{
private:
    void meta_data_block() override;

    void parse_raw_response() override;

    void parse_response_meta() override;

public:
    DirectResponse(int tcp_fd, std::vector<std::string> &headers, po::variables_map &args) :
        ICYResponse(tcp_fd, headers, args) {}
    void send_response() override;
};


#endif //RADIO_PROXY_DIRECTRESPONSE_H
