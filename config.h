#ifndef CONFIG_H
#define CONFIG_H

// interval for video capture, in seconds
//#define INTERVAL 60*60 // 60 mins
#define INTERVAL 10
//                     videoYYYY-  MM-  DD-  hh-  mm- ss
#define FILENAME "/tmp/video%04d-%02d-%02d-%02d-%02d-%02d.mjpeg"

// there will be MAX_VIDS-1 completed videos and one in progress
#define MAX_VIDS 6

#define FILENAME_LEN 255

// maximum size (in MB) that all the videos should occupy on disk
#define MAX_TOTAL_SIZE 100

#define PAGE "<html><head><title>libmicrohttpd demo</title>"\
             "</head><body>libmicrohttpd demo %d</body></html>"

#endif
