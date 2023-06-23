#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// read in file <name>
// returns:
//  - pointer to the code.  pad code with 0s up to the
//    next multiple of 4.  
//  - bytes of code in <size>
//
// fatal error open/read of <name> fails.
// 
// How: 
//    - use stat to get the size of the file.
//    - round up to a multiple of 4.
//    - allocate a buffer  --- 
//    - zero pads to a // multiple of 4.
//    - read entire file into buffer.  
//    - make sure any padding bytes have zeros.
//    - return it.   
//
// make sure to close the file descriptor (this will
// matter for later labs).
void *read_file(unsigned *size, const char *name) {
    struct stat s;
    int ret;
    if((ret = stat(name, &s)) < 0)
    {
        panic("the return code is %d.\n", ret);
        exit(-1);
    }

    *size = s.st_size;
    void *contents = calloc(sizeof(unsigned char), *size + (*size % 4));
    if (contents == NULL) {
        exit(1);
    }

    int fd = open(name, O_RDONLY);
    if(fd < 0) {
        exit(1);
    }

    if(read(fd, contents, *size) < 0) {
        exit(1);
    }
    close(fd);
    return contents;
    // FILE *file = fopen(name, "r");
    // if(!file) return 0;
    // // use ret as size
    // ret = s.st_size;
    // if((ret & 3) > 0) ret = ret / 4 + 1;
    // uint8_t *buf = calloc(ret, 4);
    // *size = ret;

    // ret = fread(buf, 1, s.st_size, file);
    // fclose(file);
    // return buf;
    // unimplemented();
}
