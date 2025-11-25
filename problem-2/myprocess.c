#include <arpa/inet.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

static volatile sig_atomic_t go = 0;

static void onusr(int s) { (void)s; go = 1; }

int try_connect(const char *host, int port)
{
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) return -1;
    struct sockaddr_in sa = {0};
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);
    inet_pton(AF_INET, host, &sa.sin_addr);
    int ret = connect(s, (struct sockaddr *)&sa, sizeof(sa));
    if (ret == 0) {
        char buf[64];
        ssize_t n = read(s, buf, sizeof(buf)-1);
        if (n > 0) { buf[n] = '\0'; printf("received: %s", buf); }
        close(s);
        return 0;
    }
    close(s);
    return -1;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "usage: %s [wait] <port>\n", argv[0]);
        return 2;
    }

    if (argc < 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        return 2;
    }
    int port = atoi(argv[1]);

    printf("myprocess pid=%d attempting connect to port %d\n", getpid(), port);
    fflush(stdout);
    int ok = try_connect("127.0.0.1", port);
    if (ok == 0) {
        printf("connect to %d: success\n", port);
        return 0;
    } else {
        perror("connect");
        printf("connect to %d: failed\n", port);
        return 1;
    }
}
