#include <unistd.h>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cerrno>

#include <thread>

#include "net.h"


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
    listen(listen_fd, 128);

    while (true) {
        sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_fd = accept(listen_fd, (sockaddr *)&client_addr, &client_addr_len);
        if (client_fd == -1) {
            perror("accept error");
            continue;
        }

        std::thread t(process_client, client_addr, client_fd);
        t.detach();
    }

    close(listen_fd);
    return 0;
}