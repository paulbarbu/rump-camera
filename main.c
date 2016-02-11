#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "httpd.h"
#include "manager.h"
#include "video.h"
#include "config.h"

static time_t start_time;

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
    struct MHD_Daemon *d;

    if (argc != 2)
    {
        printf("Port is missing, using %d\n", port);
    }
    else
    {
        port = atoi(argv[1]);
    }

	// TODO: this should be prepopulated when the program starts
	struct manager *mgr = manager_create();

	if(NULL == mgr)
	{
		perror("manager_create");
		exit(1);
	}

    d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION, port,  NULL, NULL, &httpd_serve, mgr, MHD_OPTION_END);

    if (d == NULL)
    {
        return 1;
    }

	char filename[FILENAME_LEN];
	char fullname[FILENAME_LEN];

    do
    {
    	start_time = time(NULL);
    	struct tm *utc = gmtime(&start_time);

    	int rval = snprintf(filename, FILENAME_LEN, FILENAME, 1900+utc->tm_year, 1+utc->tm_mon, utc->tm_mday, utc->tm_hour, utc->tm_min, utc->tm_sec);
    	if(rval < 0 || rval >= FILENAME_LEN)
    	{
    		perror("snprintf");
    		exit(1);
    	}

    	rval = snprintf(fullname, FILENAME_LEN, "%s%s", BASE_DIR, filename);
    	if(rval < 0 || rval >= FILENAME_LEN)
    	{
    		perror("snprintf");
    		exit(1);
    	}

    	printf("Writing to %s\n", fullname);

		//TODO: only at the end move it to start_date-end_date
		FILE *fd = fopen(fullname, "a");
		if(NULL == fd)
		{
			perror("fopen");
			exit(EXIT_FAILURE);
		}

		//TODO: this should return a status and I should sleep 1 second and retry (the file should be reopened)
		capture_video(fd, timeout_elapsed);

		if(0 != manager_add(mgr, filename, fullname, ftell(fd)/1024/1024))
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
