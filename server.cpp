#include <unistd.h>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cerrno>

#include <vector>
#include <thread>
#include <iostream>

#include <fmt/core.h>

#include "net.h"

using namespace std;

struct connection_info {
    sockaddr_in addr;
    int conn_fd;
    int client_id;
};


template <>
struct fmt::formatter<sockaddr_in> {
    constexpr auto parse(format_parse_context &ctx) {
        return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const sockaddr_in &addr, FormatContext &ctx) {
        char ip[ADDRSTR_SIZE];
        inet_ntop(AF_INET, &addr.sin_addr.s_addr, ip, sizeof(ip));
        return format_to(ctx.out(), "{}:{}", ip, ntohs(addr.sin_port));
    }
};


void process_client(sockaddr_in addr, int conn_fd);

#define LISTENQ_SIZE 128


int main()
{
    sockaddr_in server_addr;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_DEFAULT_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    int option = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(int));

    if (listen_fd == -1) pexit("socket error");

    bind(listen_fd, (sockaddr *)&server_addr, sizeof(server_addr));
    listen(listen_fd, LISTENQ_SIZE);

    std::vector<connection_info> conn_infos;
    int client_inc_id = 0;

    fd_set rset, allset;
    int max_fd = listen_fd;
    FD_ZERO(&allset);
    FD_SET(listen_fd, &allset);

    while (true) {
        rset = allset;
        int ret = select(max_fd + 1, &rset, nullptr, nullptr, nullptr);
        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            pexit("select error");
        }

        if (FD_ISSET(listen_fd, &rset)) {
            sockaddr_in client_addr;
            socklen_t client_addr_len = sizeof(client_addr);
            int conn_fd = accept(listen_fd, (sockaddr *)&client_addr, &client_addr_len);
            if (conn_fd == -1) {
                perror("accept error");
                continue;
            }

            FD_SET(conn_fd, &allset);

            if (conn_fd > max_fd) {
                max_fd = conn_fd;
            }

            conn_infos.push_back({client_addr, conn_fd, client_inc_id++});
            fmt::print("client {} ip: {} connected.\n", conn_infos.back().client_id, conn_infos.back().addr);
        }
        

        std::vector<std::vector<connection_info>::iterator> closed_conn_infos;
        for (auto it = conn_infos.begin(); it != conn_infos.end(); ++it) {
            sockaddr_in client_addr = it->addr;
            int conn_fd = it->conn_fd;
            int client_id = it->client_id;

            char buffer[BUFSIZ];
            if (FD_ISSET(conn_fd, &rset)) {
                int ret = read(conn_fd, buffer, BUFSIZ);
                if (ret == 0) {
                    close(conn_fd);
                    FD_CLR(conn_fd, &allset);
                    closed_conn_infos.push_back(it);
                } else if (ret == -1) {
                    perror("read error");
                } else {
                    for (int j = 0; j < ret; j++) {
                        buffer[j] = toupper(buffer[j]);
                    }
                    write(conn_fd, buffer, ret);
                    
                    fmt::print("client {} ip: {}, message: {}\n", client_id, client_addr, buffer);
                }
            }
        }

        

        if (!closed_conn_infos.empty()) {
            fmt::print("closed {} connections: [", closed_conn_infos.size());
            bool first = true;
            // backward erase
            for (auto it = closed_conn_infos.rbegin(); it != closed_conn_infos.rend(); ++it) {
                conn_infos.erase(*it);
                if (!first) {
                    fmt::print(", ");
                }
                fmt::print("{}", (*it)->client_id);
                first = false;
            }
            fmt::print("].\n");
        }

        fmt::print("exists {} connections.\n", conn_infos.size());
    }

    close(listen_fd);
    return 0;
}


void process_client(sockaddr_in addr, int conn_fd) {
    char buffer[BUFSIZ];
    char client_ip[ADDRSTR_SIZE];

    printf("client ip: %s, port: %d connected.\n",
           inet_ntop(AF_INET, &addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
           ntohs(addr.sin_port));

    while (true) {
        int len = read(conn_fd, buffer, sizeof(buffer));
        if (len == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("read error");
            break;
        } else if (len == 0) {
            printf("client ip: %s, port: %d closed.\n",
                   inet_ntop(AF_INET, &addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                   ntohs(addr.sin_port));
            break;
        } else {
            printf("client ip: %s, port: %d, message: %s\n",
                   inet_ntop(AF_INET, &addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
                   ntohs(addr.sin_port), buffer);
            for (int i = 0; i < len; i++) {
                buffer[i] = toupper(buffer[i]);
            }
            write(conn_fd, buffer, len);
        }
    }
    
    close(conn_fd);
}