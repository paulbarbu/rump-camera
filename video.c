#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>

#include "video.h"
#include "credentials.h"

#define DEFAULT_BUF_SIZE 65536
#define WRITE_THRESHOLD 65000

void capture_video(FILE* fd, int (*stop)())
{
    assert(fd != NULL);

    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    if(-1 == sfd)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(8888);
    inet_pton(AF_INET, "192.168.0.251", &addr.sin_addr);
    
    if(-1 == connect(sfd, (struct sockaddr*)&addr, sizeof(addr)))
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    char buf[DEFAULT_BUF_SIZE];
    size_t buf_size = 0;
    int numbytes = 0;

    char *httpreq = "GET /videostream.cgi?rate=0&user=" CAM_USER "&pwd=" CAM_PASS " HTTP/1.1\r\n\r\n"
                    "Host: 192.168.0.251:8888i\r\n";

    if(-1 == send(sfd, httpreq, strlen(httpreq), 0))
    {
        perror("send");
        exit(EXIT_FAILURE);
    }

    do
    {
        //TODO: at the first response if the HTTP response code is != 200 return error 
        //TODO: at the first response I may want to capture th boundary and to strip the rest of the headers
        numbytes = recv(sfd, buf + buf_size, DEFAULT_BUF_SIZE - buf_size, 0);
        //printf("Received %d bytes\n", numbytes);

        if(-1 == numbytes)
        {
            perror("recv");
            exit(EXIT_FAILURE);
        }

        buf_size += numbytes;
        if(buf_size >= WRITE_THRESHOLD)
        {
            if(1 != fwrite(buf, buf_size, 1, fd))
            {
                fprintf(stderr, "fwrite failed\n");
                exit(EXIT_FAILURE);
            }
            printf("Written %d bytes to file\n", buf_size);

            buf_size = 0;
        }
        //TODO: maybe flush the fd every three writes or so
    }
    while(0 != numbytes && !stop());

    if(buf_size > 0)
    {
        if(1 != fwrite(buf, buf_size, 1, fd))
        {
            fprintf(stderr, "fwrite failed\n");
            exit(EXIT_FAILURE);
        }
    }
    
    fflush(fd);

    close(sfd);
}
