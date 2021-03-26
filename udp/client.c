/*
* \brief
* a simple udp client to recv/send
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
static int flags = 0; /*function flags*/

#define PORT    5678 /*set port number*/
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


int main(int argc, char *argv[])
{
    int sockfd, send_bytes;
    char send_buf[BUFSIZE], server_ip[INET_ADDRSTRLEN];

    struct sockaddr_in server_info;
    socklen_t server_info_size = sizeof server_info;

    memset(&server_info, 0, sizeof server_info);
    server_info.sin_addr.s_addr = inet_addr(argv[1]);
    server_info.sin_port = htons(PORT);
    server_info.sin_family = AF_INET;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, protocol)) == -1)    /* create socket */
    {
        perror("talker: socket");
        return -1;
    }

    printf("please input message\n");
    scanf("%s", send_buf);

    /* send string to server */
    if ((send_bytes = sendto(sockfd, send_buf, sizeof send_buf, flags, (struct sockaddr *)&server_info, sizeof server_info)) == -1)
    {
        perror("talker: sendto");
        exit(1);
    }
    else
    {
        printf("talker: sent %s to %s\n", send_buf, argv[1]);
    }

    close(sockfd); /* client close connection */
    return 0;
}