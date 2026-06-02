/*
 * Minimal TCP listener: accepts one connection at a time and sends a courtesy
 * message. Useful as a placeholder when the MUD is down or for port checks.
 *
 * Build:  cc -O2 -o courtesy_port courtesy_port.c
 * Run:    ./courtesy_port [port]
 *         SERVER_PORT=4002 ./courtesy_port
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static const char *DEFAULT_MESSAGE =
    "\r\n"
    "Welcome to Nebbie Arcane.\r\n"
    "The game is under maintenance.\r\n"
    "Please try again later.\r\n"
    "\r\n";

static volatile sig_atomic_t keep_running = 1;
static int listen_fd = -1;

static void on_signal(int sig)
{
    (void)sig;
    keep_running = 0;
    if (listen_fd >= 0) {
        close(listen_fd);
        listen_fd = -1;
    }
}

static int parse_port(const char *text, int fallback)
{
    char *end = NULL;
    long value;

    if (!text || !*text) {
        return fallback;
    }
    value = strtol(text, &end, 10);
    if (*end != '\0' || value < 1 || value > 65535) {
        fprintf(stderr, "Invalid port: %s\n", text);
        exit(1);
    }
    return (int)value;
}

int main(int argc, char **argv)
{
    const char *env_port = getenv("SERVER_PORT");
    int port = parse_port(
        argc > 1 ? argv[1] : (env_port ? env_port : "4002"),
        4002
    );
    const char *message = getenv("COURTESY_MESSAGE");
    if (!message || !*message) {
        message = DEFAULT_MESSAGE;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    int yes = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons((uint16_t)port);

    if (bind(listen_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(listen_fd);
        return 1;
    }
    if (listen(listen_fd, 8) < 0) {
        perror("listen");
        close(listen_fd);
        return 1;
    }

    printf("Courtesy server listening on port %d\n", port);
    fflush(stdout);

    while (keep_running) {
        struct sockaddr_in client;
        socklen_t client_len = sizeof(client);
        int client_fd = accept(listen_fd, (struct sockaddr *)&client, &client_len);
        if (client_fd < 0) {
            if (errno == EINTR || !keep_running) {
                break;
            }
            perror("accept");
            break;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client.sin_addr, ip, sizeof(ip));
        printf("Connection from %s:%d\n", ip, ntohs(client.sin_port));
        fflush(stdout);

        size_t len = strlen(message);
        if (write(client_fd, message, len) < 0) {
            perror("write");
        }
        shutdown(client_fd, SHUT_WR);
        close(client_fd);
    }

    if (listen_fd >= 0) {
        close(listen_fd);
    }
    puts("Stopped.");
    return 0;
}
