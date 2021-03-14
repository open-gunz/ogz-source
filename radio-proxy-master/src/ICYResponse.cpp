#include "ICYResponse.h"

ICYResponse::ICYResponse(int tcp_fd, std::vector<std::string> &headers, po::variables_map &args) {
    this->args = args;
    this->tcp_fd = tcp_fd;
    this->headers = headers;
    this->meta = args["meta"].as<std::string>() == "yes";
    metadata_interval = meta_interval();
}

int ICYResponse::meta_interval() {
    /* search for icy-metaint header among headers */
    std::string interval;
    bool found = false;
    for (std::string &header : headers) {
        if (header.find("icy-metaint") == std::string::npos) {
            continue;
        }
        interval = header;
        found = true;
        break;
    }
    if (!found) {
        return -1;
    }
    /* parse size from icy-metaint header */
    std::string token = interval.substr(interval.find(':') + 1, interval.size() - 1);
    std::stringstream stream(token);
    int size = -1;
    if (!(stream >> size)) {
        std::cerr << "invalid metadata size, switching to no metadata" << std::endl;
    }
    return size;
}
