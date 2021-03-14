#ifndef RADIO_PROXY_ICYRESPONSE_H
#define RADIO_PROXY_ICYRESPONSE_H

#include <vector>
#include <string>
#include <boost/program_options/variables_map.hpp>
#include <iostream>
#include <sstream>
#include "err.h"

namespace po = boost::program_options;

class ICYResponse {
protected:
    const int BUF_SIZE = 1500;
    std::vector<std::string> headers;
    po::variables_map args;
    bool meta{};
    bool meta_send{};
    std::string data;
    std::string meta_data;
    int tcp_fd{};
    int metadata_interval{};

    int meta_interval();

    virtual void meta_data_block() = 0;

    virtual void parse_raw_response() = 0;

    virtual void parse_response_meta() = 0;

public:
    ICYResponse(int, std::vector<std::string>&, po::variables_map&);

    ICYResponse() = default;

    virtual void send_response() = 0;
};
#endif //RADIO_PROXY_ICYRESPONSE_H
