#ifndef CONFIG_H
#define CONFIG_H

#define BASE_DIR "/tmp/"
#define BASE_FILENAME "video"
#define FILE_EXT ".mjpeg"

// interval for video capture, in seconds
//#define INTERVAL 60*60 // 60 mins
#define INTERVAL 10

// please keep the names sortable
//                         nameYYYY-  MM-  DD-  hh-  mm- ss
#define FILENAME BASE_FILENAME"%04d-%02d-%02d-%02d-%02d-%02d"FILE_EXT

// there will be MAX_VIDS-1 completed videos and one in progress
#define MAX_VIDS 6

#define FILENAME_LEN 256

// maximum size (in MB) that all the videos should occupy on disk
#define MAX_TOTAL_SIZE 100L

#define LIST_PAGE "<!DOCTYPE html><html><head><title>rumpcamera</title>" \
             "</head><body><ol>%s</ol></body></html>"

#define LIST_ITEM "<li><a href=\"%s\">%s (%ld MB)</a></li>"

#endif
