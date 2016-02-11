#ifndef HTTPD_H_
#define HTTPD_H_

#include <microhttpd.h>

int httpd_serve(void *cls, struct MHD_Connection *connection, const char *url, const char *method,
                    const char *version, const char *upload_data, size_t *upload_data_size, void **ptr);

#endif
