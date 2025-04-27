#include <sys/types.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stddef.h>

int main() {

    struct addrinfo hints = {0};
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_RAW;

    struct addrinfo *res = NULL;

    getaddrinfo("ff02::1", "0", &hints, &res);

    return 0;
}