#define _DEFAULT_SOURCE

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>


#include "manager.h"

struct manager
{
	struct video vids[MAX_VIDS];
	size_t num;
} global_mgr;


void _video_create(struct video *v, char *name, char *fullname, long int size);
int _manager_remove_oldest(struct manager *m);
int _manager_clean_size(struct manager *m);
long int _total_size(struct manager *m);
int _video_filter(const struct dirent *d);

struct manager *manager_create()
{
	global_mgr.num = 0;

	struct stat sbuf;
	struct dirent **namelist;

	//I can benefit from the fact that the videos have sortable names by default
	int numdirs = scandir(BASE_DIR, &namelist, _video_filter, alphasort);
	if(numdirs > 0)
	{
		int start = numdirs-MAX_VIDS+1;
		if(start < 0)
		{
			// if there are less than MAX_VIDS-1 videos, just use them all
			start = 0;
		}

		for(int i=start; i<numdirs; ++i)
		{
			char fullname[FILENAME_LEN];
			snprintf(fullname, FILENAME_LEN, "%s%s", BASE_DIR, namelist[i]->d_name);

			if(-1 == stat(fullname, &sbuf))
			{
				perror("stat");
			}
			else
			{
				manager_add(&global_mgr, namelist[i]->d_name, fullname, sbuf.st_size/1024/1024);
				free(namelist[i]);
			}
		}

		for(int i=0; i<numdirs-MAX_VIDS+1; ++i)
		{
			free(namelist[i]);
		}

		free(namelist);
	}

	//assert(global_mgr.num == MAX_VIDS-1);

	return &global_mgr;
}

void manager_release(struct manager *m)
{
}

int manager_add(struct manager *m, char *filename, char *fullname, long int size)
{
	int ret = 0;

	//add the new video in the queue
	struct video v;
	_video_create(&v, filename, fullname, size);

	m->vids[m->num] = v;
	m->num += 1;

	if(m->num >= MAX_VIDS)
	{
		ret = _manager_remove_oldest(m);
	}

	ret = !ret && _manager_clean_size(m);

	return ret;
}

void manager_print(struct manager *m)
{
	if(m->num < 1)
	{
		printf("No videos yet\n");
		return;
	}

	printf("From newest to oldest, %ld videos:\n", m->num);

	for(int i=m->num-1; i>=0; --i)
	{
		printf("%d: %s (%ld MB)\n", i, m->vids[i].fullname, m->vids[i].size);
	}
}

int manager_count(struct manager *m)
{
	return m->num;
}

void manager_video(struct manager *m, int n, struct video **v)
{
	*v = NULL;

	if(n < m->num)
	{
		*v = &m->vids[n];
	}
}

struct video *manager_find(struct manager *m, const char *name)
{
	for(int i=m->num-1; i>=0; --i)
	{
		if(0 == strncmp(name, m->vids[i].name, FILENAME_LEN))
		{
			return &m->vids[i];
		}
	}

	return NULL;
}

int _manager_remove_oldest(struct manager *m)
{
	if(m->num == 0)
	{
		printf("No items to remove\n");
		return 0;
	}

	printf("Removing %s\n", m->vids[0].fullname);
	// delete the oldest video
	if(-1 == unlink(m->vids[0].fullname))
	{
		perror("unlink");
		return 1;
	}

	//shift the other videos to the left
	for(int i=1; i<MAX_VIDS; ++i)
	{
		m->vids[i-1] = m->vids[i];
	}

	m->num -= 1;

	return 0;
}

int _manager_clean_size(struct manager *m)
{
	int ret = 0;
	long int total = _total_size(m);
	long int average_size = total / m->num;

	//printf("Max. size: %ld MB\nTotal size: %ld MB\nAverage size: %ld MB\n", MAX_TOTAL_SIZE, total, average_size);

	if(total + average_size > MAX_TOTAL_SIZE)
	{
		ret = _manager_remove_oldest(m);
	}

	return ret;
}

void _video_create(struct video *v, char *name, char *fullname, long int size)
{
	v->size = size;
	strncpy(v->name, name, FILENAME_LEN);
	strncpy(v->fullname, fullname, FILENAME_LEN);
}

long int _total_size(struct manager *m)
{
	long int sum = 0;
	for(int i=0; i < m->num; ++i)
	{
		sum += m->vids[i].size;
	}

	return sum;
}

int _video_filter(const struct dirent *d)
{
	//check the start of the name
	if(0 == strncmp(d->d_name, BASE_FILENAME, strlen(BASE_FILENAME)))
	{
		//check the extension
		int ext_len = strlen(FILE_EXT);
		if(0 == strncmp(d->d_name + strlen(d->d_name) - ext_len, FILE_EXT, ext_len))
		{
			return 1; // keep this entry
		}
	}

	return 0; // discard this entry
}
