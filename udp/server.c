/*
* \brief
* a simple udp server to recv/send
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
static int flags = 0; /* function flags */

#define PORT    5678 /* set port number */
#define BUFSIZE 256
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
    struct sockaddr_in server_info, client_info;
    socklen_t client_info_size;
    int recv_bytes, sockfd;
    char recv_buf[BUFSIZE], client_ip[INET_ADDRSTRLEN];

    memset(&server_info, 0, sizeof server_info);
    server_info.sin_addr.s_addr = INADDR_ANY;
    server_info.sin_port = htons(PORT);
    server_info.sin_family = AF_INET;

    client_info_size = sizeof client_info;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, protocol)) == -1)   /* create socket */
    {
        perror("listener: socket");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *)&server_info, sizeof server_info) == -1)  /* bind socket and port */
    {
        close(sockfd);
        perror("listener: bind");
        return -1;
    }

    printf("listener: waiting to recvfrom...\n");

    while (1)
    {
        /* handle for recv_bytes <= 0 */
        if ((recv_bytes = recvfrom(sockfd, recv_buf, sizeof recv_buf, flags, (struct sockaddr *)&client_info, &client_info_size)) <= 0)
        {
            if (recv_bytes == 0)
            {
                printf("client close the connection\n");
            }
            else
            {
                perror("recvfrom");
                return -1;
            }
        }
        else
        {
            /* print the string from client */
            printf("listener: got packet from %s\n",
                   inet_ntop(client_info.sin_family,
                             (struct sockaddr *)&client_info.sin_addr
                             , client_ip, INET_ADDRSTRLEN));
            printf("listener: packet contains \"%s\"\n", recv_buf);
        }
    }

    close(sockfd);

    return 0;

}