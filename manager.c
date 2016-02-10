#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "manager.h"

struct video
{
	char filename[FILENAME_LEN];
	long int size;	
};

struct manager
{
	struct video vids[MAX_VIDS];
	size_t num;
} global_mgr;


void _video_create(struct video *v, char *filename, long int size);
int _manager_remove_oldest(struct manager *m);
int _manager_clean_size(struct manager *m);
long int _total_size(struct manager *m);


struct manager *manager_create()
{
	global_mgr.num = 0;
	return &global_mgr;
}

void manager_release(struct manager *m)
{
}

int manager_add(struct manager *m, char *filename, long int size)
{		
	int ret = 0;

	//add the new video in the queue	
	struct video v;
	_video_create(&v, filename, size);
	
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
		printf("%d: %s (%ld MB)\n", i, m->vids[i].filename, m->vids[i].size);
	}
}

int _manager_remove_oldest(struct manager *m)
{
	if(m->num == 0)
	{
		printf("No items to remove\n");
		return 0;
	}

	printf("Removing %s\n", m->vids[0].filename);
	// delete the oldest video
	if(-1 == unlink(m->vids[0].filename))
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

	printf("Total size: %ld\nAverage size: %ld\n", total, average_size);

	if(total + average_size > MAX_TOTAL_SIZE)
	{
		ret = _manager_remove_oldest(m);
	}

	return ret;
}

void _video_create(struct video *v, char *filename, long int size)
{
	v->size = size;
	strncpy(v->filename, filename, FILENAME_LEN);
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

