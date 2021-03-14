#include <thread>
#include "ClientResponse.h"

/* get meta-data blockf from icy-response */
void ClientResponse::meta_data_block() {
    /* meta-data block size */
    char temp = 0;
    int read_size;
    read_size = read(tcp_fd, &temp, 1);
    if (read_size < 0) {
        syserr("reading from tcp socket");
    } else if (read_size == 0) {
        std::cerr << "meta-data block size not found on socket" << std::endl;
        exit(1);
    }
    /* read temp * 16 bytes */
    int byte_cnt = 0;
    char ch;
    int metadata_length = temp * 16;
    while (byte_cnt != metadata_length && (read_size = read(tcp_fd, &ch, 1))) {
        if (read_size < 0) {
            syserr("reading from the tcp socket");
        }
        meta_data += ch;
        byte_cnt++;
    }
    if (byte_cnt != metadata_length) {
        std::cerr << "metadata is not sent completely";
        exit(1);
    }
}

void ClientResponse::parse_response_meta() {
    /* response contains meta-data, needs to be parsed */
    int read_size;
    char ch;
    meta_send = false;
    while ((read_size = read(tcp_fd, &ch, 1))) {
        if (read_size < 0) {
            syserr("reading from the tcp socket");
        }
        data += ch;
        byte_count++;
        /* interval is finised, meta block starts */
        if (byte_count == metadata_interval) {
            meta_data_block();
            byte_count = 0;
            if (meta_data.length()) {
                meta_send = true;
                break;
            }
        }
        /* buffer is full, head to sending to client */
        if ((int)data.length() >= BUF_SIZE) {
            break;
        }
    }
}

void ClientResponse::parse_raw_response() {
    /* no meta no fun, just store raw to be sent to clients */
    int read_size;
    char ch;
    while ((read_size = read(tcp_fd, &ch, 1))) {
        if (read_size < 0) {
            syserr("reading from tcp socket");
        }
        data += ch;
        if ((int)data.length() >= BUF_SIZE) {
            break;
        }
    }
}

void ClientResponse::send_response() {
    /* establish server */
    UDPServer udpServer(args);
    /* if we didn't ask for meta or if server didn't send it */
    if (!meta || metadata_interval == -1) {
        if (metadata_interval != -1) {
            std::cerr << "proxy didn't ask for meta but server sent, exit" << std::endl;
            exit(1);
        }
        udpServer.connect();
        std::thread t(&UDPServer::update_clients, &udpServer);
        while (true) {
            parse_raw_response();
            udpServer.deliver_data(data, 0);
            data.clear();
        }
    } else {
        udpServer.connect();
        std::thread t(&UDPServer::update_clients, &udpServer);
        while (true) {
            parse_response_meta();
            udpServer.deliver_data(data, 0);
            data.clear();
            if (meta_send) {
                udpServer.deliver_data(meta_data, 1);
                meta_data.clear();
            }
        }
    }
}