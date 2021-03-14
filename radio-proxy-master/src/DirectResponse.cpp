#include "DirectResponse.h"

void DirectResponse::meta_data_block() {
    char temp = 0;
    int read_size;
    read_size = read(tcp_fd, &temp, 1);
    if (read_size < 0) {
        syserr("reading from tcp socket");
    } else if (read_size == 0) {
        std::cerr << "meta-data block size not found on socket" << std::endl;
        exit(1);
    }
    int byte_count = 0;
    char ch;
    int metadata_length = temp * 16;
    while (byte_count != metadata_length && (read_size = read(tcp_fd, &ch, 1))) {
        if (read_size < 0) {
            syserr("reading from the tcp socket");
        }
        std::cerr << ch;
        byte_count++;
    }
    if (byte_count != metadata_length) {
        std::cerr << "metadata is not sent completely";
        exit(1);
    }
}

void DirectResponse::parse_raw_response() {
    int read_size;
    char ch;
    while ((read_size = read(tcp_fd, &ch, 1))) {
        if (read_size < 0) {
            syserr("reading from tcp socket");
        }
        std::cout << ch;
    }
}

void DirectResponse::parse_response_meta() {
    int read_size;
    int byte_count = 0;
    char ch;
    while ((read_size = read(tcp_fd, &ch, 1))) {
        if (read_size < 0) {
            syserr("reading from tcp socket");
        }
        std::cout << ch;
        byte_count++;
        if (byte_count == metadata_interval) {
            meta_data_block();
            byte_count = 0;
        }
    }
}

void DirectResponse::send_response() {
    if (!meta || metadata_interval == -1) {
        if (metadata_interval != -1) {
            std::cerr << "proxy didn't ask for meta but server sent, exit" << std::endl;
            exit(1);
        }
        parse_raw_response();
    } else {
        parse_response_meta();
    }
}