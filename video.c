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
#include "config.h"

#define DEFAULT_BUF_SIZE 65536
#define WRITE_THRESHOLD 65000
#define OK_RESPONSE "HTTP/1.1 200 OK"
#define MAX_WRITE_COUNT 3

#define xstr(a) str(a)
#define str(a)

int capture_video(FILE* fd, int (*stop)())
{
    assert(fd != NULL);
    assert(stop != NULL);

    int checked_http_response = 0;
    int write_count = 0;
    int sfd = socket(AF_INET, SOCK_STREAM, 0);

    if(-1 == sfd)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(IP_CAMERA_PORT);
    inet_pton(AF_INET, IP_CAMERA_ADDR, &addr.sin_addr);

    if(-1 == connect(sfd, (struct sockaddr*)&addr, sizeof(addr)))
    {
        perror("connect");
        return 1;
    }

    char buf[DEFAULT_BUF_SIZE];
    size_t buf_size = 0;
    int numbytes = 0;

    char *httpreq = "GET /videostream.cgi?rate=0&user=" CAM_USER "&pwd=" CAM_PASS " HTTP/1.1\r\n\r\n"
                    "Host: " IP_CAMERA_ADDR ":" xstr(IP_CAMERA_PORT) "\r\n";

    if(-1 == send(sfd, httpreq, strlen(httpreq), 0))
    {
        perror("send");
        return 1;
    }

    do
    {
        numbytes = recv(sfd, buf + buf_size, DEFAULT_BUF_SIZE - buf_size, 0);
        //printf("Received %d bytes\n", numbytes);

        if(-1 == numbytes)
        {
            perror("recv");
            return 1;
        }

        //verify the first response
        if(!checked_http_response)
        {
        	char *response_header_ending = memchr(buf, '\r', numbytes);
        	if(NULL != response_header_ending)
        	{
				if(0 != strncmp(buf, OK_RESPONSE, strlen(OK_RESPONSE)))
				{
					//print the part after "HTTP/1.1 ", until the end of the line
					fwrite(buf+9, 1, response_header_ending-buf-9, stderr);
					fwrite("\n", 1, 1, stderr);
					return 1;
				}

				checked_http_response = 1;
        	}
        }

        buf_size += numbytes;
        if(buf_size >= WRITE_THRESHOLD)
        {
            if(1 != fwrite(buf, buf_size, 1, fd))
            {
                fprintf(stderr, "fwrite failed\n");
                return 1;
            }
            //printf("Written %ld bytes to file\n", buf_size);

            write_count+=1;
            if(write_count >= MAX_WRITE_COUNT)
            {
            	fflush(fd);
            	write_count = 0;
            }

            buf_size = 0;
        }
    }
    while(0 != numbytes && !stop());

    if(buf_size > 0)
    {
        if(1 != fwrite(buf, buf_size, 1, fd))
        {
            fprintf(stderr, "fwrite failed\n");
            return 1;
        }
    }

    fflush(fd);
    close(sfd);

    return 0;
}
