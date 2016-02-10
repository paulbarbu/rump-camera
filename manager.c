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
	size_t num_created;
} global_mgr;


void video_create(struct video *v, char *filename, long int size);
int manager_clean_old(struct manager *m);


struct manager *manager_create()
{
	global_mgr.num_created = 0;
	return &global_mgr;
}

void manager_release(struct manager *m)
{
}

int manager_add(struct manager *m, char *filename, long int size)
{		
	//add the new video in the queue	
	struct video v;
	video_create(&v, filename, size);
	
	m->vids[m->num_created] = v;	
	m->num_created += 1;
	
	return manager_clean_old(m);
}

void manager_print(struct manager *m)
{
	if(m->num_created < 1)
	{
		printf("No videos yet\n");
		return;
	}
	
	printf("From newest to oldest, %ld videos:\n", m->num_created);
		
	for(int i=m->num_created-1; i>=0; --i)
	{
		printf("%d: %s (%ld MB)\n", i, m->vids[i].filename, m->vids[i].size);
	}
}

int manager_clean_old(struct manager *m)
{
	if(m->num_created >= MAX_VIDS)
	{
		m->num_created = MAX_VIDS-1;
		
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
	}
	
	return 0;
}

void video_create(struct video *v, char *filename, long int size)
{
	v->size = size;
	strncpy(v->filename, filename, FILENAME_LEN);
}