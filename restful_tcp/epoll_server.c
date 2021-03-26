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
#include <netdb.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

/*****************************************************************************
* Private types/enumerations/variables/define
****************************************************************************/
#define PORT 80
#define BUFSIZE 1024
#define BACKLOG 1
#define PROTOCOL 0
#define FLAGS    0
#define MAXEVENTS 64
#define REQSIZE  3
#define REQUEST "GET"

/*****************************************************************************
 * Private function declaration
 ****************************************************************************/
/**
 *  \brief  Set server info.
 *  \param[in]  None
 *
 *  \return socket number
 *
 *  This function set server info and bind socket and port.
 */
static int create_and_bind()
{
    int listener, optval = 1;
    struct sockaddr_in server_info;

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

    return listener;
}

/**
 *  \brief  Make socket becomes non-blocking socket.
 *  \param[in]  process_socket the socket will be set
 *
 *  \return -1  the socket set fail
 *  \return 0   the socket set success
 * 
 *  This function make socket becomes non-blocking socket.
 */
static int make_non_blocking_socket(int process_socket)
{
    int flags, s;

    if ((flags = fcntl(process_socket, F_GETFL, 0)) == -1)
    {
        perror("fcntl");
        return -1;
    }

    flags |= O_NONBLOCK;

    if ((s = fcntl(process_socket, F_SETFL, flags)) == -1)
    {
        perror("fcntl");
        return -1;
    }

    return 0;
}
/*****************************************************************************
 * Public variables
 ****************************************************************************/

/*****************************************************************************
* Public functions
****************************************************************************/


int main()
{
    char webpage[] =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n" /* return json format */
        "Content-Length: 97\r\n"
        "Connection: keep-alive\r\n" /* close connection after responsing */
        "cache-control: public, max-age=10\r\n" /* cache 10 sec */
        "\r\n"
        "[\r\n"
        "  {\r\n"
        "    \"id\": 1,\r\n"
        "    \"title\":\"post 1\"\r\n"
        "  },\r\n"
        "  {\r\n"
        "    \"id\": 2,\r\n"
        "    \"title\":\"post 2\"\r\n"
        "  }\r\n"
        "]";

    int epoll_socket, listener, new_fd, recv_bytes;
    struct sockaddr_in client_info;
    socklen_t client_info_size = sizeof client_info;

    struct epoll_event event;
    struct epoll_event *event_queue;

    char client_ip[INET_ADDRSTRLEN], recv_buf[BUFSIZE];

    if ((listener = create_and_bind()) == -1)
    {
        return -1;
    }

    if (make_non_blocking_socket(listener) == -1) /* make listener becomes non-blocking socket */
    {
        return -1;
    }

    if (listen(listener, BACKLOG) == -1)
    {
        close(listener);
        perror("listen");
        return -1;
    }

    printf("server is listening\n");

    if ((epoll_socket = epoll_create1(0)) == -1) /* create a epoll_socket listen all sockets */
    {
        perror("epoll_create");
        return -1;
    }

    event.data.fd = listener;
    event.events = EPOLLIN | EPOLLET; /* event can be read, event trigger mode = edge trigger */

    if (epoll_ctl(epoll_socket, EPOLL_CTL_ADD, listener, &event) == -1) /* add listener in epoll_socket */
    {
        perror("epoll_ctl");
        return -1;
    }

    event_queue = calloc(MAXEVENTS, sizeof event); /* set the epoll array */

    while (1)
    {
        int num_ready;

        if ((num_ready = epoll_wait(epoll_socket, event_queue, MAXEVENTS, -1)) == -1) /* return the number of ready socket */
        {
            perror("epoll_wait");
            return -1;
        }

        for (int i = 0; i < num_ready; i++)
        {
            if ((event_queue[i].events & EPOLLERR) || (event_queue[i].events & EPOLLHUP) || (!(event_queue[i].events & EPOLLIN)))
            {
                /* have error, client close connection, or no data can be read*/
                fprintf(stderr, "epoll error\n");
                close(event_queue[i].data.fd);
                continue;
            }
            else if (listener == event_queue[i].data.fd)
            {
                /* handle listener */
                while (1)
                {
                    if ((new_fd = accept(listener, (struct sockaddr *)&client_info, &client_info_size)) == -1)
                    {
                        if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
                        {
                            /* all socket request finish */
                            break;
                        }
                        else
                        {
                            perror("accept");
                            break;
                        }
                    }
                    else /* there a new incoming connection */
                    {
                        printf("new connection from %s on socket %d\n", inet_ntop(AF_INET,
                                (struct sockaddr *)&client_info.sin_addr,
                                client_ip, sizeof client_ip)
                               , new_fd);

                        if (make_non_blocking_socket(new_fd) == -1)  //set socket to non-blocking socket
                        {
                            return -1;
                        }

                        event.data.fd = new_fd;
                        event.events = EPOLLIN | EPOLLET;

                        if (epoll_ctl(epoll_socket, EPOLL_CTL_ADD, new_fd, &event) == -1) //add socket to epoll_socket
                        {
                            perror("epoll_ctl");
                            return -1;
                        }
                    }
                }

                continue;
            }
            else
            {
                /* handle the connection recv buffer*/
                int done = 0;

                while (1)
                {
                    if ((recv_bytes = recv(event_queue[i].data.fd, recv_buf, sizeof recv_buf, FLAGS)) == -1) //read recv buffer until recv_bytes = 0
                    {
                        if (errno != EAGAIN)    /* errno = EAGAIN, represent buffer don't have data */
                        {
                            perror("read");
                            done = 1;
                        }

                        break;
                    }
                    else if (recv_bytes == 0) /* recv client close connection */
                    {
                        done = 1;
                        break;
                    }
                    else
                    {
                        /* if recv GET req, send json */
                        if (strncmp(recv_buf, REQUEST, REQSIZE) == 0)
                        {
                            if (send(event_queue[i].data.fd, webpage, strlen(webpage), FLAGS) == -1)
                            {
                                perror("send");
                            }
                        }
                    }
                }

                if (done)
                {
                    /* remove the socket in the epoll set after closing connection */
                    printf("Closed connection on descriptor %d\n", event_queue[i].data.fd);
                    close(event_queue[i].data.fd);
                }
            }
        }
    }

    free(event_queue);

    close(listener);

    return 0;
}