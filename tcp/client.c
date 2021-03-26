/*
* \brief
* a simple tcp client to recv/send
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
static int flags    = 0;    /* function flags */

#define PORT    1234 /* set port number */
#define BUFSIZE 100
/*****************************************************************************
 * Private function declaration
 ****************************************************************************/

/*****************************************************************************
 * Public variables
 ****************************************************************************/

/*****************************************************************************
* Public functions
****************************************************************************/

int main(int argc, char *argv[])
{
    int sockfd, send_bytes;
    char send_buf[BUFSIZE], server_ip[INET_ADDRSTRLEN];
    struct sockaddr_in server_info;

    if (argc != 2)
    {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&server_info, 0, sizeof server_info);
    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = inet_addr(argv[1]);
    server_info.sin_port = htons(PORT);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, protocol)) == -1)
    {
        perror("client: socket");
        return -1;
    }

    if ((connect(sockfd, (struct sockaddr *)&server_info, sizeof server_info)) == -1) /* connect to server, send syn */
    {
        perror("client connect");
        return -1;
    }

    printf("client: connecting to %s\n", inet_ntop(AF_INET, (struct sockaddr *)&server_info.sin_addr, server_ip, INET_ADDRSTRLEN));


    while (1)
    {
        printf("please input message\n");
        scanf("%s", send_buf);

        if (strcmp(send_buf, "exit") == 0)  /* input exit to close connection */
        {
            printf("client will close the connection\n");
            break;
        }
        else
        {
            if ((send_bytes = send(sockfd, send_buf, sizeof send_buf, flags)) == -1)
            {
                perror("send");    /* send string to server */
            }
        }
    }

    close(sockfd);   /* client close connection, send fin/ack */
    return 0;
}