// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>

#include "libunix.h"
#include <sys/stat.h>

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
	"ttyUSB",	// linux
	"cu.SLAB_USB", // mac os
    // if your system uses another name, add it.
	0
};

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    // unimplemented();
    const char *file_name = d->d_name;
    for(int i = 0; i<2; i++) {
        const char* prefix = ttyusb_prefixes[i];
        if(strstr(file_name, prefix) != NULL) {
            return 1;
        }
    }
    return 0;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    struct dirent **namelist;
    int n = scandir("/dev", &namelist, filter, alphasort);    
    // unimplemented();
    if(n == 0) {
        perror("no file");
        exit(1);
    } else if(n > 1) {
        perror("too many files");
        exit(1);
    }
    char* path = malloc(strlen(namelist[0]->d_name) + 1 + 5);
    strcpy(path, "/dev/");
    strcat(path, namelist[0]->d_name) ;

    free(namelist[0]);
    free(namelist);
    return path;
// the strdup function (CPP version)
//     char * __strdup(const char *s)
// {
//    size_t  len = strlen(s) +1;
//    void *new = malloc(len);
//    if (new == NULL)
//       return NULL;
//    return (char *)memecpy(new,s,len);
// }
 
}

static char *get_dev_filepath(const char *name) {
    int len = strlen("/dev/") + strlen(name) + 1;
    char *path = malloc(len);
    sprintf(path, "/dev/%s", name);
    path[len-1] = 0;
    return path;

}

static int modificationsort_asc(const struct dirent **d1, const struct dirent **d2) {
    struct stat s1, s2;
    char *d1_name = get_dev_filepath((*d1)->d_name), 
         *d2_name = get_dev_filepath((*d2)->d_name);
    if (stat(d1_name, &s1) < 0 || stat(d2_name, &s2) < 0) {
        printf("modificationsort_asc: damn stat failed\n");
        exit(1);
    }
    printf("(%s: %ld) - (%s: %ld)\n", d1_name, s1.st_mtim.tv_sec, d2_name, s2.st_mtim.tv_sec);
    return s1.st_mtim.tv_sec - s2.st_mtim.tv_sec; // compare seconds since epoch, ignore nanoseconds
}

static int modificationsort_desc(const struct dirent **d1, const struct dirent **d2) {
    return -modificationsort_asc(d1, d2);
}

static char *get_filename(const struct dirent *d) {
    return strdup(get_dev_filepath((char *)d->d_name));
}

static int do_find_ttyusb(int (p)(const struct dirent **, const struct dirent **), char **name) {
    int nummatches;
    struct dirent **entries;
    if ((nummatches = scandir("/dev", &entries, filter, p)) < 0) {
        printf("scanner pains\n");
        exit(1);
    }
    // check that there's at least one element
    if (nummatches == 0) {
        printf("sadge %d matches\n", nummatches);
        exit(1);
    }
    *name = get_filename(entries[0]);
    return nummatches;
}
// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) {
    char *ttyusb_name;
    do_find_ttyusb(modificationsort_desc, &ttyusb_name);
    return ttyusb_name;
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
    char *ttyusb_name;
    do_find_ttyusb(modificationsort_asc, &ttyusb_name);
    return ttyusb_name;
}