// simple program to run a command when any file that is "interesting" in a directory
// changes.
// e.g., 
//      cmd-watch make
// will run make at each change.
//
// This should use the scandir similar to how you did `find_ttyusb`
//
// key part will be implementing two small helper functions (useful-examples/ will be 
// helpful):
//  - static int pid_clean_exit(int pid);
//  - static void run(char *argv[]);
//
#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    // unimplemented();
    char *suffixes[] = { ".c", ".h", ".S", "Makefile", 0 };
    const char *file_name = d->d_name;
   
    int index = 0;
    for(int i =0 ;i < sizeof(suffixes); i++){
        if(strstr(d->d_name, suffixes)) 
            return 1;
    }
        
    return 0;
}

// scan the files in "./" (you can extend this) for those
// that match the suffixes in <suffixes> and check  if any
// have changed since the last time.
int check_activity(void) {
    char *suffixes[] = { ".c", ".h", ".S", "Makefile", 0 };
    const char *dirname = ".";
    int changed_p = 0;

    static time_t last_mtime = 0;   // store last modification time.

    // unimplemented();
    int nummatches;
    struct dirent **entries;
    if ((nummatches = scandir(dirname, &entries, filter, alphasort)) < 0) {
        printf("scanner pains\n");
        exit(1);
    }
    // check that there's at least one element
    if (nummatches == 0) {
        printf("sadge %d matches\n", nummatches);
        return changed_p;
    }
    
    int len = sizeof(entries)/sizeof(entries[0]);
    for(int i = 0;i<len;i++) {
        struct stat s1;
        char *file_name = get_dev_filepath((*entries)->d_name);
    }
    
    if (stat(file_name, &s1) < 0) {
        printf("check_activity: file_name stat failed\n");
        exit(1);
    }


    // return 1 if anything that matched <suffixes>
    return changed_p;
}

// synchronously wait for <pid> to exit.  returns 1 if it exited
// cleanly (via exit(0)), 0 otherwise.
static int pid_clean_exit(int pid) {
    // unimplemented();
    int status;
	debug("testing <waitpid(pid, &status, 0)>\n");
	if(waitpid(pid, &status, 0) < 0)
		sys_die(waitpid, waitpid failed?);
	status = WEXITSTATUS(status);
	debug("\tkid=%d exited with %d\n\n", pid, status);
	if(status == 0)
        return 1;
    else
        return 0;
}

// simple helper to print null terminated vector of strings.
static void print_argv(char *argv[]) {
    assert(argv[0]);

	fprintf(stderr, "about to exec =<%s ", argv[0]);
	for(int i =1; argv[i]; i++)
		fprintf(stderr, " %s", argv[i]);
	fprintf(stderr, ">\n");
}


// fork/exec <argv> and wait for the result: print an error
// and exit if the kid crashed or exited with an error (a non-zero
// exit code).
static void run(char *argv[]) {
    // unimplemented();

}

int main(int argc, char *argv[]) {
    if(argc < 2)
        die("cmd-watch: not enough arguments\n");
        
    unimplemented();
    return 0;
}
