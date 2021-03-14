#ifndef RADIO_PROXY_UDPSERVER_H
#define RADIO_PROXY_UDPSERVER_H


#include <string>
#include <netinet/in.h>
#include <boost/program_options/variables_map.hpp>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <iostream>
#include <mutex>
#include <thread>
#include <poll.h>
#include "err.h"
#include "UDPClient.h"

namespace po = boost::program_options;

class UDPServer {
private:
    /* server config */
    po::variables_map args;
    std::string local_dotted_address;
    in_port_t local_port;
    int timeout;
    /* socket */
    int sock{};
    /* clients which is served is kept */
    std::vector<UDPClient> clients;
    /* buffer to communicate */
    static const int B_SIZE = 1500;
    std::string buffer;
    /* mutex for exclusive access to clients list*/
    std::mutex mutex;
    void update_client_time(UDPClient udpClient, bool discover);
    /* response types */
    std::string IAM_response();
    static std::string data_response(std::string&, bool meta);
    /* Ban clients who reached out their timeout */
    void ban_clients();

public:
    explicit UDPServer(po::variables_map&);
    void connect();
    void deliver_data(std::string&, bool meta);

    [[noreturn]] void update_clients();
};


#endif //RADIO_PROXY_UDPSERVER_H
