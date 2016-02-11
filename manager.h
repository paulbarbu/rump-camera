#ifndef MANAGER_H
#define MANAGER_H

#include "config.h"

struct manager;

struct video
{
	char name[FILENAME_LEN];
	char fullname[FILENAME_LEN];
	long int size;
};

struct manager *manager_create();
void manager_release(struct manager *m);
int manager_add(struct manager *m, char *filename, char *fullname, long int size);
int manager_count(struct manager *m);
void manager_video(struct manager *m, int n, struct video **v);
void manager_print(struct manager *m);
struct video *manager_find(struct manager *m, const char *name);

#endif
