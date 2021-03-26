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
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

/*****************************************************************************
* Private types/enumerations/variables/define
****************************************************************************/
#define PORT 1234
#define BUFSIZE 1024
#define BACKLOG 10
#define PROTOCOL 0
#define FLAGS    0

/*****************************************************************************
 * Private function declaration
 ****************************************************************************/

/*****************************************************************************
 * Public variables
 ****************************************************************************/

/*****************************************************************************
* Public functions
****************************************************************************/

int main()
{
    fd_set master, read_fds;
    int fd_max, recv_bytes, optval = 1;
    int listener, new_fd;
    struct sockaddr_in server_info, client_info;
    socklen_t client_info_size = sizeof client_info;

    char client_ip[INET_ADDRSTRLEN], recv_buf[BUFSIZE];

    FD_ZERO(&master); /* clear the fds in master and read_fds set */
    FD_ZERO(&read_fds);

    memset(&server_info, 0, sizeof server_info);
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(PORT);

    if ((listener = socket(AF_INET, SOCK_STREAM, PROTOCOL)) == -1)
    {
        perror("socket");
        return -1;
    }

    if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof optval) == -1)
    {
        perror("set socket");
        return -1;
    }

    if (bind(listener, (struct sockaddr *)&server_info, sizeof server_info) == -1)
    {
        close(listener);
        perror("bind");
        return -1;
    }

    if (listen(listener, BACKLOG) == -1)
    {
        perror("listen");
        return -1;
    }


    printf("server is listening\n");

    FD_SET(listener, &master);
    fd_max = listener; /* update master set and fd_max */

    while (1)
    {
        read_fds = master; /* update read_fds */

        if (select(fd_max + 1, &read_fds, NULL, NULL, NULL) == -1) /* listen connection whether is ready */
        {
            perror("select");
            return -1;
        }

        for (int i = 0; i <= fd_max; i++)
        {
            if (FD_ISSET(i, &read_fds)) /* if the socket in read_fd */
            {
                if (i == listener) /* if the socket is listener, handle new connection */
                {
                    if ((new_fd = accept(listener, (struct sockaddr *)&client_info, &client_info_size)) == -1) /* accept new connection */
                    {
                        perror("accept");
                        return -1;
                    }
                    else
                    {
                        FD_SET(new_fd, &master); /* update master set */

                        if (new_fd > fd_max)
                        {
                            fd_max = new_fd;
                        }

                        printf("new connection from %s on socket %d\n", inet_ntop(AF_INET,
                                (struct sockaddr *)&client_info.sin_addr, client_ip, sizeof client_ip)
                               , new_fd);
                    }
                }
                else
                {
                    if ((recv_bytes = recv(i, recv_buf, sizeof recv_buf, FLAGS)) <= 0) /* if the socket is connection, handle recv */
                    {
                        if (recv_bytes == 0)  /* handle recv <= 0 */
                        {
                            printf("server: socket %d close the connection\n", i);
                        }
                        else
                        {
                            perror("recv");
                        }

                        close(i);
                        FD_CLR(i, &master);
                    }
                    else
                    {
                        printf("client: received '%s' from socket %d\n", recv_buf, i);
                    }
                }
            }
        }
    }

    close(listener);

    return 0;
}