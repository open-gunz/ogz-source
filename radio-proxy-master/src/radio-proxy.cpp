#include "radio-proxy.h"
#include "ICYResponse.h"
#include "ClientResponse.h"
#include "DirectResponse.h"

namespace po = boost::program_options;

int tcp_fd;

void signal_handler(int signum) {
    (void)signum;
    if (close(tcp_fd) == -1) {
        syserr("tcp socket close");
    }
    exit(0);
}

po::variables_map validate_args(int argc, char **argv) {
    po::options_description description("usage");
    description.add_options()
            ("host,h", po::value<std::string>()->required(), "Host")
            ("resource,r", po::value<std::string>()->required(), "Resource")
            ("port,p", po::value<in_port_t>()->required(), "Port")
            ("meta,m", po::value<std::string>()->default_value("no"), "Meta-data")
            ("timeout,t", po::value<int>()->default_value(5), "Wait timeout")
            ("Port,P", po::value<in_port_t>()->default_value(0), "multicast port")
            ("address,B", po::value<std::string>()->default_value("n/a"), "group address")
            ("timeoutclient,T", po::value<int>()->default_value(5), "timeout client");
    po::variables_map vm;
    try {
        po::store(po::parse_command_line(argc, argv, description), vm);
        po::notify(vm);
    } catch (po::error &e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        std::cerr << description << std::endl;
        exit(1);
    }
    return vm;
}

int initialize_tcp(std::string host, std::string port) {
    /* split host and port */
    int sock;
    struct addrinfo addr_hints{};
    struct addrinfo *addr_result;
    int err;
    /* 'converting' host/port in string to struct addrinfo */
    memset(&addr_hints, 0, sizeof(struct addrinfo));
    addr_hints.ai_family = AF_INET; // IPv4
    addr_hints.ai_socktype = SOCK_STREAM;
    addr_hints.ai_protocol = IPPROTO_TCP;
    err = getaddrinfo(&host[0], &port[0], &addr_hints, &addr_result);
    if (err == EAI_SYSTEM) { // system error
        syserr("getaddrinfo: %s", gai_strerror(err));
    }
    else if (err != 0) { // other error (host not found, etc.)
        fatal("getaddrinfo: %s", gai_strerror(err));
    }
    /* initialize socket according to getaddrinfo results */
    sock = socket(addr_result->ai_family, addr_result->ai_socktype, addr_result->ai_protocol);
    if (sock < 0) {
        syserr("tcp socket");
    }
    /* connect socket to the server */
    if (connect(sock, addr_result->ai_addr, addr_result->ai_addrlen) < 0) {
        syserr("tcp socket connect");
    }
    freeaddrinfo(addr_result);
    return sock;
}

std::string icy_request(const po::variables_map &args) {
    /* create icy-request */
    std::vector<std::string> req_headers;
    req_headers.push_back("GET " + args["resource"].as<std::string>() + " HTTP/1.0");
    if (args["meta"].as<std::string>() == "yes") {
        req_headers.emplace_back("Icy-MetaData:1");
    }
    req_headers.emplace_back("Host: " + args["host"].as<std::string>());
    req_headers.emplace_back("Connection: close");
    /* finish */
    std::string end("\r\n");
    std::string request;
    /* add all headers to request */
    for (std::string &header : req_headers) {
        request += header;
        request += end;
    }
    request += end;
    return request;
}

void normalize_header(std::string &header) {
    for (char &x : header) {
        x = (char)tolower(x);
    }
}

void get_headers(std::vector<std::string> &headers) {
    /* get all headers from response and store them*/
    std::string header;
    int read_size;
    char ch;
    while ((read_size = read(tcp_fd, &ch, 1))) {
        if (read_size < 0) {
            syserr("reading from socket");
        }
        char _back = header.empty() ? ' ' : header.back();
        header += ch;
        /* end of the current header */
        if (_back == '\r' && ch == '\n') {
            headers.push_back(header);
            header.clear();
        }
        if (!headers.empty() && headers.back() == "\r\n") {
            break;
        }
    }
    /* normalize to be able to compare with no doubt */
    for (std::string &_header : headers) {
        normalize_header(_header);
    }
}

void check_ok_response(std::string &first_header) {
    if (first_header.find("200") == std::string::npos) {
        std::cerr << "not ok response: " << first_header << std::endl;
        exit(1);
    }
}

void wait_response(po::variables_map &args) {
    /* get response headers and validate 200*/
    std::vector<std::string> headers;
    get_headers(headers);
    check_ok_response(headers[0]);
    /* check if should send to clients */
    ICYResponse *icyResponse;
    if (!args["Port"].defaulted()) {
        icyResponse = new ClientResponse(tcp_fd, headers, args);
    } else {
        icyResponse = new DirectResponse(tcp_fd, headers, args);
    }
    icyResponse->send_response();
}

void check_args(po::variables_map &args) {
    if (args["timeout"].as<int>() <= 0) {
        std::cerr << "timeout can't be zero" << std::endl;
        exit(1);
    }
}

int main(int argc, char **argv) {
    /* set signal handler */
    std::signal(SIGINT, signal_handler);
    /* parse and validate program arguments */
    po::variables_map args = validate_args(argc, argv);
    check_args(args);
    /* open tcp socket */
    tcp_fd = initialize_tcp(args["host"].as<std::string>(),
            std::to_string(args["port"].as<in_port_t>()));
    /* create request and send it to the server */
    std::string request = icy_request(args);
    if (write(tcp_fd, &request[0], request.length()) < 0) {
        syserr("writing to tcp socket");
    }
    /* set timeout to read from socket */
    struct timeval tv{};
    tv.tv_sec = args["timeout"].as<int>();
    tv.tv_usec = 0;
    if (setsockopt(tcp_fd, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval)) < 0) {
        syserr("timeout option tcp socket");
    }
    /* proceed to getting response */
    wait_response(args);
    return 0;
}
