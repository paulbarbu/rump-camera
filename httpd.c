#include <microhttpd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "config.h"
#include "manager.h"

#define MAX_ITEM_LEN 2*FILENAME_LEN + 16 + strlen(LIST_ITEM)

int _sprint_video_list(char **text, struct manager *mgr)
{
	struct video *v = NULL;
	size_t buffer_len = 0;
	size_t buffer_size = 1024;
	char *buffer = (char*) malloc(buffer_size);
	char item[MAX_ITEM_LEN];

	if(NULL == buffer)
	{
		perror("malloc");
		return 1;
	}

	for(int i=manager_count(mgr)-1; i>=0; --i)
	{
		manager_video(mgr, i, &v);

		if(v != NULL)
		{
			snprintf(item, MAX_ITEM_LEN, LIST_ITEM, v->name, v->name, v->size);
			int item_len = strlen(item);

			// enlarge the text buffer is needed
			if(item_len + buffer_len > buffer_size)
			{
				buffer_size += 2*item_len;
				char *new_text = realloc(buffer, buffer_size);

				if(NULL == new_text)
				{
					free(buffer);
					buffer = NULL;
					perror("realloc");
					return 1;
				}
				buffer = new_text;
			}

			strncat(buffer, item, item_len);
			buffer_len += item_len;
		}
	}

	*text = (char*) malloc(buffer_size);

	if(NULL == *text)
	{
		perror("malloc");
		return 1;
	}

    snprintf(*text, buffer_size, LIST_PAGE, buffer);

    free(buffer);

    return 0;
}

int _httpd_list_files(struct MHD_Connection *connection, void *data)
{
    assert(data != NULL);

	int ret = MHD_NO;
    struct MHD_Response *response;
    struct manager *mgr = (struct manager *)data;
    char *text = NULL;

    if(0 != _sprint_video_list(&text, mgr))
    {
    	return MHD_NO;
    }

    response = MHD_create_response_from_buffer(strlen(text), (void*) text, MHD_RESPMEM_MUST_FREE);

    if(NULL == response)
    {
    	return MHD_NO;
    }

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

int _httpd_try_download(struct MHD_Connection *connection, void *data,
		const char *url, int (*failure_handler)(struct MHD_Connection *connection, void *data))
{
	assert(data != NULL);

	int ret = MHD_NO;
    struct MHD_Response *response;

    struct manager *mgr = (struct manager *)data;
    struct video *v = NULL;

    // strip the leading slash from the url
    if(NULL == (v = manager_find(mgr, url+1)))
    {
    	return failure_handler(connection, data);
    }

    int fd = open(v->fullname, O_RDONLY);

    if(-1 == fd)
    {
    	perror("open");
    	return failure_handler(connection, data);
    }

    struct stat sbuf;

    if(-1 == fstat(fd, &sbuf))
    {
    	perror("fstat");
    	close(fd);
    	return failure_handler(connection, data);
    }

    response = MHD_create_response_from_fd(sbuf.st_size, fd);

    ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
}

int httpd_serve(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
		const char *version, const char *upload_data, size_t *upload_data_size, void **ptr)
{
    static int dummy;

    if (0 != strcmp(method, "GET"))
    {
        return MHD_NO;
    }

    if (&dummy != *ptr)
    {
        // The first time only the headers are valid, do not respond yet
        *ptr = &dummy;
        return MHD_YES;
    }

    *ptr = NULL; // clear context pointer

    printf("URL: %s\n", url);

    char *download = strstr(url, "/"BASE_FILENAME);

    // if the pattern is not found or is not at the beginning
    // just display the list of videos, otherwise try to download the video
    if(NULL == download || url != download)
    {
    	return _httpd_list_files(connection, cls);
    }

    return _httpd_try_download(connection, cls, url, _httpd_list_files);
}
