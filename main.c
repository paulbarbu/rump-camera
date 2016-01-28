#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "video.h"

#define PAGE "<html><head><title>libmicrohttpd demo</title>"\
             "</head><body>libmicrohttpd demo %d</body></html>"

static int ahc_echo(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                    const char *version, const char *upload_data, size_t *upload_data_size, void **ptr)
{
    static int calls = 0;
    static int dummy;
    const char *page = cls;
    struct MHD_Response *response;
    int ret;

    if (0 != strcmp(method, "GET"))
        return MHD_NO; /* unexpected method */

    if (&dummy != *ptr)
    {
        /* The first time only the headers are valid,
        do not respond in the first round... */
        *ptr = &dummy;
        return MHD_YES;
    }
    
    if (0 != *upload_data_size)
        return MHD_NO; /* upload data in a GET!? */

    *ptr = NULL; /* clear context pointer */
    char text[500];
    sprintf(text, page, calls);
    response = MHD_create_response_from_buffer(strlen(text), (void*) text, MHD_RESPMEM_MUST_COPY);
    /*response = MHD_create_response_from_buffer(strlen(page), (void*) page, MHD_RESPMEM_PERSISTENT);*/

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    ++calls;
    return ret;
}

int main(int argc, char ** argv) 
{
    int port = 12312;
    struct MHD_Daemon * d;

    if (argc != 2)
    {
        printf("Port is missing, using 12312\n");
    }
    else
    {
        port = atoi(argv[1]);
    }

    d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, port,  NULL, NULL, &ahc_echo, PAGE, MHD_OPTION_END);

    if (d == NULL)
        return 1;

    //TODO: keep a series of log files

    //TODO: the name of the file should be the start date
    //TODO: only at the end move it to start_date-end_date
    FILE *fd = fopen("/tmp/video.mjpeg", "a");
    if(NULL == fd)
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }

    //TODO: this should return a status and I should sleep 1 second and retry (the file should be reopened)
    capture_video(fd);

    fclose(fd);
    /*for(int i=0; i<100; ++i)*/
    /*{*/
        /*printf("%d\n", i);*/
        /*sleep(1);*/
    /*}*/

    getchar ();
    MHD_stop_daemon(d);

    return 0;
}
