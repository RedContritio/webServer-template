#include <unistd.h>
#include <cstdio>
#include <cctype>
#include <cstdlib>
#include <cerrno>

#include "net.h"


void sys_error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main()
{
    int lfd = 0;
    int cfd = 0;

    char buffer[BUFSIZ];
    char client_ip[ADDRSTR_SIZE];

    sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_DEFAULT_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    lfd = socket(AF_INET, SOCK_STREAM, 0);

    if (lfd == -1)
    {
        sys_error("socket error");
    }
    bind(lfd, (sockaddr *)&server_addr, sizeof(server_addr));
    listen(lfd, 128);

    client_addr_len = sizeof(client_addr);
    cfd = accept(lfd, (sockaddr *)&client_addr, &client_addr_len);
    if (cfd == -1)
    {
        sys_error("accept error");
    }

    printf("client ip: %s, port: %d\n",
           inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, client_ip, sizeof(client_ip)),
           ntohs(client_addr.sin_port));

    while (true) {
        int len = read(cfd, buffer, sizeof(buffer));
        if (len == -1) {
            if (errno == EINTR) {
                continue;
            }
            sys_error("read error");
        } else if (len == 0) {
            printf("client closed\n");
            break;
        } else {
            write(STDOUT_FILENO, buffer, len);
            write(STDOUT_FILENO, "\n", 1);
            for (int i = 0; i < len; i++) {
                buffer[i] = toupper(buffer[i]);
            }
            write(cfd, buffer, len);
        }
    }

    close(lfd);
    close(cfd);
    return 0;
}