#include "UDPServer.h"

UDPServer::UDPServer(po::variables_map &args) {
    this->args = args;
    local_port = args["Port"].as<uint16_t>();
    local_dotted_address = args["address"].as<std::string>();
    this->timeout = args["timeoutclient"].as<int>();
}

void UDPServer::connect() {
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        syserr("udp socket");
    }
    struct sockaddr_in local_address{};
    if (!args["address"].defaulted()) {
        struct ip_mreq ipMreq{};
        ipMreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (inet_aton(&local_dotted_address[0], &ipMreq.imr_multiaddr) == 0) {
            std::cerr << "ERROR: inet_aton - invalid multicast address" << std::endl;
            exit(1);
        }
        if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void *)&ipMreq, sizeof(ipMreq)) < 0) {
            syserr("setsockopt add membership");
        }
    }
    local_address.sin_family = AF_INET;
    local_address.sin_addr.s_addr = htonl(INADDR_ANY);
    local_address.sin_port = htons(local_port);
    if (bind(sock, (struct sockaddr*)&local_address, sizeof(local_address)) < 0) {
        syserr("udp socket bind");
    }
}

[[noreturn]] void UDPServer::update_clients() {
    /* wait for request from client */
    struct sockaddr_in sender{};
    socklen_t fromlen = sizeof(sender);
    buffer.resize(B_SIZE);
    ssize_t rcv_len;
    while (true) {
        ssize_t total = 0;
        /* wait until enough data is present on the socket */
        while (total < 4) {
            rcv_len = recvfrom(sock, &buffer[total], B_SIZE - total, 0,
                    (struct sockaddr *) &sender, &fromlen);
            if (rcv_len < 0) {
                syserr("recvfrom udp");
            }
            total += rcv_len;
        }
        /* convert type and length to host byte order */
        uint16_t type;
        uint16_t length;
        memcpy(&type, &buffer[0], 2);
        memcpy(&length, &buffer[2], 2);
        type = ntohs(type);
        length = ntohs(length);
        mutex.lock();
        /* before updating times, check if someone already should banned */
        ban_clients();
        if (type == 1 && !length) { /* DISCOVER */
            /* update client time */
            update_client_time(UDPClient(sender), true);
            /* send IAM response back */
            std::string response = IAM_response();
            if (sendto(sock, &response[0], response.length(), 0,
                    (struct sockaddr*)&sender, sizeof(sender)) != (ssize_t)response.length()) {
                syserr("sento udp iam");
            }
        } else if (type == 3 && !length) { /* KEEPALIVE */
            update_client_time(UDPClient(sender), false);
        } else {
            std::cerr << "UNKNOWN REQUEST" << std::endl;
        }
        mutex.unlock();
    }
}

void UDPServer::deliver_data(std::string &data, bool meta) {
    std::string response = data_response(data, meta);
    mutex.lock();
    ban_clients();
    for (UDPClient &udpClient : clients) {
        if (!udpClient.is_banned()) {
            struct sockaddr_in sender = udpClient.get_addr();
            if (sendto(sock, &response[0], response.length(), 0, (struct sockaddr*)&sender,
                       sizeof(sender)) != (ssize_t)response.length()) {
                syserr("sento");
            }
        }
    }
    mutex.unlock();
}

void UDPServer::update_client_time(UDPClient udpClient, bool discover) {
    for (UDPClient &udpClient1 : clients) {
        if (udpClient1.is_equal(udpClient)) {
            if (!udpClient1.is_banned()) {
                auto now = std::chrono::system_clock::now();
                udpClient1.set_last_request(std::chrono::system_clock::to_time_t(now));
            }
            return;
        }
    }
    if (discover) {
        auto now = std::chrono::system_clock::now();
        udpClient.set_last_request(std::chrono::system_clock::to_time_t(now));
        clients.push_back(udpClient);
    }
}

std::string UDPServer::IAM_response() {
    std::string response;
    /* type */
    uint16_t type = htons(2);
    /* length of the information about radio and itself */
    uint16_t length = args["host"].as<std::string>().length() +
            args["resource"].as<std::string>().length();
    std::string info = args["host"].as<std::string>() + args["resource"].as<std::string>();
    /* 4 bytes for header and the rest for the info */
    response.resize(4 + length);
    length = htons(length);
    /* assembly message to client */
    memcpy(&response[0], &type, sizeof(type));
    memcpy(&response[sizeof(type)], &length, sizeof(length));
    memcpy(&response[sizeof(type) + sizeof(length)], &info[0], info.length());
    return response;
}

void UDPServer::ban_clients() {
    for (UDPClient &udpClient : clients) {
        if (!udpClient.is_banned()) {
            time_t start = udpClient.get_last_request();
            time_t end = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            time_t elapsed_seconds = end - start;
            if (elapsed_seconds > timeout) {
                udpClient.ban();
            }
        }
    }
}

std::string UDPServer::data_response(std::string &data, bool meta) {
    std::string response;
    /* type of the response*/
    uint16_t type;
    type = meta ? htons(6) : htons(4);
    /* length of the data */
    uint16_t length = data.length();
    /* 4 bytes for header and the rest for the data */
    response.resize(4 + length);
    length = htons(length);
    /* assembly message to client */
    memcpy(&response[0], &type, sizeof(type));
    memcpy(&response[sizeof(type)], &length, sizeof(length));
    memcpy(&response[sizeof(type) + sizeof(length)], &data[0], data.length());
    return response;
}