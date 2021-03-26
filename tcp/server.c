/*
* \brief
* a simple tcp server to recv/send
*
* \copyright
* Copyright (C) MOXA Inc. All rights reserved.
* This software is distributed under the terms of the
* MOXA License. See the file COPYING-MOXA for details.
*
* \date 2021/02/26
* First release
* \author Alan Lan
*/

/*****************************************************************************
* Include files
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

/*****************************************************************************
* Private types/enumerations/variables/define
****************************************************************************/
static int protocol = 0;
static int flags    = 0; /*function flags*/

#define PORT    1234 /*set port number*/
#define BACKLOG 10   /*the number of permitted connection*/
#define BUFSIZE 256
/*****************************************************************************
 * Private function declaration
 ****************************************************************************/
/**
 *	\brief	remove all zombie process.
 *	\param[in]	s	sigchld_handler
 * 							
 *	\return None
 *
 *	This function remove all zombie process.
 */
static void sigchld_handler(int s)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

/*****************************************************************************
 * Public variables
 ****************************************************************************/

/*****************************************************************************
* Public functions
****************************************************************************/

int main()
{
    int optval = 1;
    struct sigaction sa;

    int sockfd, new_fd, recv_bytes;
    struct sockaddr_in server_info, client_info;
    char client_ip[INET_ADDRSTRLEN], recv_buf[BUFSIZE];
    socklen_t client_info_size = sizeof client_info;

    sa.sa_handler = sigchld_handler; /* remove all zombie process */
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        return -1;
    }

    memset(&server_info, 0, sizeof server_info); /* set server info */
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(PORT);


    if ((sockfd = socket(AF_INET, SOCK_STREAM, protocol)) == -1)
    {
        perror("server: socket");
        return -1;
    }

    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == -1)
    {
        perror("setsockopt");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *)&server_info, sizeof server_info) == -1)
    {
        close(sockfd);
        perror("server: bind");
        return -1;
    }

    /* server listen */
    if (listen(sockfd, BACKLOG) == -1)
    {
        perror("listen");
        return -1;
    }

    printf("server: waiting for connections...\n");

    while (1)
    {
        /* accept new connection, return syn/ack */
        if ((new_fd = accept(sockfd, (struct sockaddr *)&client_info, &client_info_size)) == -1)
        {
            perror("accept");
            return -1;
        }

        /* IP turn into the format can be read */
        printf("server: got connection from %s\n", inet_ntop(AF_INET, (struct sockaddr *)&client_info.sin_addr, client_ip, INET_ADDRSTRLEN));

        /* fork() == 0 represent child process handle each connection, fork() > 0 represent parent process handle server listen */
        if (fork() == 0)
        {
            close(sockfd);

            if ((recv_bytes = recv(new_fd, recv_buf, sizeof recv_buf, flags)) <= 0)
            {
                if (recv_bytes == 0)
                {
                    printf("client close the connection\n");
                }
                else
                {
                    perror("recv");
                    return -1;
                }
            }
            else
            {
                printf("server recv %s from socket %d\n", recv_buf, new_fd);
            }

            close(new_fd); /* connection end */
            return 0;
        }

        close(new_fd);
    }

    return 0;
}
