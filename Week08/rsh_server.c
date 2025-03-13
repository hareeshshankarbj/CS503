#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "dshlib.h"
#include "rshlib.h"

int boot_server(char *ifaces, int port) {
    int svr_socket;
    struct sockaddr_in server_addr;
    int enable = 1;

    svr_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (svr_socket < 0) {
        perror("socket");
        return ERR_RDSH_COMMUNICATION;
    }

    if (setsockopt(svr_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
        perror("setsockopt");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ifaces, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid interface: %s\n", ifaces);
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    if (bind(svr_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    if (listen(svr_socket, 5) < 0) {
        perror("listen");
        close(svr_socket);
        return ERR_RDSH_COMMUNICATION;
    }

    return svr_socket;
}

int stop_server(int svr_socket) {
    return close(svr_socket);
}

int send_message_eof(int cli_socket) {
    int bytes_sent = send(cli_socket, &RDSH_EOF_CHAR, 1, 0);
    if (bytes_sent != 1) {
        return ERR_RDSH_COMMUNICATION;
    }
    return OK;
}


int send_message_string(int cli_socket, char *buff) {
    int total = strlen(buff) + 1;  // include null byte if desired
    int bytes_sent = send(cli_socket, buff, total, 0);
    if (bytes_sent != total)
        return ERR_RDSH_COMMUNICATION;
    return send_message_eof(cli_socket);
}

int rsh_execute_pipeline(int cli_sock, command_list_t *clist) {
    int num_cmds = clist->num;
    int prev_fd = -1;
    pid_t pids[CMD_MAX];
    int pipefd[2];
    int exit_code = 0;

    for (int i = 0; i < num_cmds; i++) {
        if (i < num_cmds - 1) {
            if (pipe(pipefd) < 0) {
                perror("pipe");
                return ERR_RDSH_CMD_EXEC;
            }
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            return ERR_RDSH_CMD_EXEC;
        }

        if (pid == 0) {  
            if (i == 0) {
                dup2(cli_sock, STDIN_FILENO);
            } else {
                dup2(prev_fd, STDIN_FILENO);
                close(prev_fd);
            }
            
            if (i == num_cmds - 1) {
                dup2(cli_sock, STDOUT_FILENO);
                dup2(cli_sock, STDERR_FILENO);
            } else {
                dup2(pipefd[1], STDOUT_FILENO);
                close(pipefd[0]);
                close(pipefd[1]);
            }
            execvp(clist->commands[i].argv[0], clist->commands[i].argv);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {  
            if (i < num_cmds - 1) {
                close(pipefd[1]);
            }
            if (i > 0) {
                close(prev_fd);
            }
            if (i < num_cmds - 1) {
                prev_fd = pipefd[0];
            }
            pids[i] = pid;
        }
    }

    for (int i = 0; i < num_cmds; i++) {
        int status;
        waitpid(pids[i], &status, 0);
        if (i == num_cmds - 1) {
            exit_code = WEXITSTATUS(status);
        }
    }

    return exit_code;
}


int exec_client_requests(int cli_socket) {
    char recv_buff[ARG_MAX];
    int rc = OK;
    while (1) {
        
        int bytes = recv(cli_socket, recv_buff, ARG_MAX, 0);
        if (bytes <= 0) {
        
            break;
        }
      
        recv_buff[bytes] = '\0';

        if (strlen(recv_buff) == 0)
            continue;

        if (strcmp(recv_buff, "exit") == 0) {
        
            break;
        }
        if (strcmp(recv_buff, "stop-server") == 0) {
       
            rc = OK_EXIT;
            break;
        }

        command_list_t clist;
        rc = build_cmd_list(recv_buff, &clist);
        if (rc < 0) {
            char err_msg[128];
            snprintf(err_msg, sizeof(err_msg), CMD_ERR_RDSH_EXEC);
            send_message_string(cli_socket, err_msg);
            continue;
        }
        rc = rsh_execute_pipeline(cli_socket, &clist);
        free_cmd_list(&clist);

        send_message_eof(cli_socket);
    }
    return rc;
}

int process_cli_requests(int svr_socket) {
    int rc = 0;
    while (1) {
        int cli_socket = accept(svr_socket, NULL, NULL);
        if (cli_socket < 0) {
            perror("accept");
            return ERR_RDSH_COMMUNICATION;
        }
        rc = exec_client_requests(cli_socket);
        close(cli_socket);
        if (rc == OK_EXIT)
            break;
    }
    return rc;
}

int start_server(char *ifaces, int port, int is_threaded) {
    (void)is_threaded; 
    int svr_socket = boot_server(ifaces, port);
    if (svr_socket < 0)
        return svr_socket;

    int rc = process_cli_requests(svr_socket);
    stop_server(svr_socket);
    return rc;
}
