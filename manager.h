#ifndef MANAGER_H
#define MANAGER_H

#include "config.h"

struct manager;

struct manager *manager_create();
void manager_release(struct manager *m);
int manager_add(struct manager *m, char *filename, long int size);
void manager_print(struct manager *m);

#endif