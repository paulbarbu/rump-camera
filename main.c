#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "manager.h"
#include "video.h"
#include "config.h"

static time_t start_time;

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

int timeout_elapsed()
{
	time_t current_time = time(NULL);

	if(difftime(current_time, start_time) > INTERVAL)
	{
		printf("Interval %d elapsed\n", INTERVAL);
		return 1;
	}

	return 0;
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
    {
        return 1;
    }

	char filename[FILENAME_LEN];

	// TODO: this should be prepopulated when the program starts
	struct manager *mgr = manager_create();

	if(NULL == mgr)
	{
		perror("manager_create");
		exit(1);
	}

    do
    {
    	start_time = time(NULL);
    	struct tm *utc = gmtime(&start_time);

    	//TODO: watch the dimension of the videos

    	int rval = snprintf(filename, FILENAME_LEN, FILENAME, 1900+utc->tm_year, 1+utc->tm_mon, utc->tm_mday, utc->tm_hour, utc->tm_min, utc->tm_sec);
    	if(rval < 0 || rval >= FILENAME_LEN)
    	{
    		perror("snprintf");
    		exit(1);
    	}

    	printf("Writing to %s\n", filename);

		//TODO: only at the end move it to start_date-end_date
		FILE *fd = fopen(filename, "a");
		if(NULL == fd)
		{
			perror("fopen");
			exit(EXIT_FAILURE);
		}

		//TODO: this should return a status and I should sleep 1 second and retry (the file should be reopened)
		capture_video(fd, timeout_elapsed);

		if(0 != manager_add(mgr, filename, ftell(fd)/1024/1024))
		{
			perror("manager_add");
			exit(1);
		}
		manager_print(mgr);

		fclose(fd);
    }
    while(1);

    manager_release(mgr);

    MHD_stop_daemon(d);

    return 0;
}
