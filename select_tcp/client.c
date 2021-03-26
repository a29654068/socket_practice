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
#define PORT     1234
#define BUFSIZE  1024
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

int main(int argc, char *argv[])
{
    int sockfd, send_bytes;
    struct sockaddr_in server_info;
    char server_ip[INET_ADDRSTRLEN], send_buf[BUFSIZE];
    socklen_t server_info_size = sizeof server_info;

    if (argc != 2)
    {
        fprintf(stderr, "usage: client hostname\n");
        return -1;
    }

    memset(&server_info, 0, sizeof server_info);
    server_info.sin_addr.s_addr = inet_addr(argv[1]);
    server_info.sin_port = htons(PORT);
    server_info.sin_family = AF_INET;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, PROTOCOL)) == -1) /* create new socket */
    {
        perror("client: socket");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&server_info, sizeof server_info) == -1)  /* connect to server */
    {
        close(sockfd);
        perror("client: connect");
        return -1;
    }

    /* IP convert to the format can be read */
    printf("client: connecting to %s\n", inet_ntop(AF_INET, (struct sockaddr *)&server_info.sin_addr, server_ip, sizeof server_ip));

    while (1)
    {
        printf("please input message\n");
        scanf("%s", send_buf);

        if (strcmp(send_buf, "exit") == 0) /* input exit close connection */
        {
            printf("client will close...\n");
            close(sockfd);  /* close connection, send fin/ack to server */
            break;
        }
        else
        {
            if (send(sockfd, send_buf, sizeof send_buf, FLAGS) == -1)
            {
                perror("send");
            }
        }
    }

    return 0;
}