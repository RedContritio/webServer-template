#include "net.h"
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <thread>

const int CLIENT_HELLO_DELAY_MS = 1000;

void sys_error(const char *msg)
{
    perror(msg);
    exit(1);
}

void create_client(const char *dest_ip, const int dest_port, const int client_id) {
    char buffer[BUFSIZ];

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_DEFAULT_PORT);
    inet_pton(AF_INET, dest_ip, &server_addr.sin_addr.s_addr);

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1) {
        sys_error("socket error");
    }

    int ret = connect(cfd, (sockaddr *)&server_addr, sizeof(server_addr));
    if (ret != 0) {
        sys_error("connect error");
    }

    int serial_id = 0;
    while (true) {
        int len = sprintf(buffer, "hello, server! client id: %d, serial id: %d", client_id, serial_id++);
        write(cfd, buffer, len);
        ret = read(cfd, buffer, sizeof(buffer));
        if (ret == 0) {
            perror("server closed.\n");
            break;
        }
        write(STDOUT_FILENO, buffer, ret);
        write(STDOUT_FILENO, "\n", 1);
        usleep(CLIENT_HELLO_DELAY_MS * 1000);
    }

    close(cfd);
}

int main() {
    const char *dest_ip = "10.68.39.78";
    const int dest_port = SERVER_DEFAULT_PORT;

    int client_count = 2;
    int client_delay = CLIENT_HELLO_DELAY_MS * 1000 / client_count;
    std::thread *clients = new std::thread[client_count];
    for (int i = 0; i < client_count; i++) {
        clients[i] = std::thread(create_client, dest_ip, dest_port, i);
        usleep(client_delay);
    }

    for (int i = 0; i < client_count; i++) {
        clients[i].join();
    }
    return 0;
}
