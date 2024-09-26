#include "net.h"
#include <unistd.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>

void sys_error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main() {
    char buffer[BUFSIZ];

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_DEFAULT_PORT);
    inet_pton(AF_INET, "10.68.39.78", &server_addr.sin_addr.s_addr);

    int cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1) {
        sys_error("socket error");
    }

    int ret = connect(cfd, (sockaddr *)&server_addr, sizeof(server_addr));
    if (ret != 0) {
        sys_error("connect error");
    }

    write(cfd, "hello", 5);
    sleep(1);
    ret = read(cfd, buffer, sizeof(buffer));
    write(STDOUT_FILENO, buffer, ret);
    write(STDOUT_FILENO, "\n", 1);

    close(cfd);
    return 0;
}
