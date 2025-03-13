#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "dshlib.h"
#include "rshlib.h"


int client_cleanup(int cli_socket, char *cmd_buff, char *rsp_buff, int rc) {
    if (cli_socket > 0) {
        close(cli_socket);
    }
    if (cmd_buff) {
        free(cmd_buff);
    }
    if (rsp_buff) {
        free(rsp_buff);
    }
    return rc;
}

int start_client(char *server_ip, int port) {
    int sock;
    struct sockaddr_in server_addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return ERR_RDSH_CLIENT;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid server address: %s\n", server_ip);
        close(sock);
        return ERR_RDSH_CLIENT;
    }

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return ERR_RDSH_CLIENT;
    }

    return sock;
}


int exec_remote_cmd_loop(char *address, int port)
{
    int sock = start_client(address, port);
    if (sock < 0)
        return ERR_RDSH_CLIENT;

    char send_buff[1024];
    char recv_buff[RDSH_COMM_BUFF_SZ];
    int bytes_sent, bytes_received;
   

    while (1) {
        printf("rdsh> ");
        if (fgets(send_buff, sizeof(send_buff), stdin) == NULL)
            break;
        send_buff[strcspn(send_buff, "\n")] = '\0';
        if (strlen(send_buff) == 0)
            continue;

        bytes_sent = send(sock, send_buff, strlen(send_buff) + 1, 0);
        if (bytes_sent != (int) (strlen(send_buff) + 1)) {
            perror("send");
            return client_cleanup(sock, NULL, NULL, ERR_RDSH_COMMUNICATION);
        }
        if (strcmp(send_buff, "exit") == 0)
            break;

        while (1) {
            bytes_received = recv(sock, recv_buff, RDSH_COMM_BUFF_SZ, 0);
            if (bytes_received < 0) {
                perror("recv");
                return client_cleanup(sock, NULL, NULL, ERR_RDSH_COMMUNICATION);
            }
            if (bytes_received == 0)
                break; 
            int is_last_chunk = (recv_buff[bytes_received-1] == RDSH_EOF_CHAR);
            if (is_last_chunk)
                recv_buff[bytes_received-1] = '\0';
            else
                recv_buff[bytes_received] = '\0';

            printf("%s", recv_buff);
            if (is_last_chunk)
                break;
        }
    }

    return client_cleanup(sock, NULL, NULL, OK);
}
