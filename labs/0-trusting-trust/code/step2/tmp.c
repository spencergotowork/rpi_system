#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define error(args...) do { fprintf(stderr, ##args); exit(1);  } while(0)

static void compile(char *program, char *outname) {
    FILE *fp = fopen("./temp-out.c", "w");
    assert(fp);
    fprintf(fp, "%s", program);
    fclose(fp);


    /*****************************************************************
     * Step 1:
     */
 
    // match on the start of the login() routine:
    static char login_sig[] = "int login(char *user) {";

    // and inject an attack for "ken":
    static char login_attack[] = "if(strcmp(user, \"ken\") == 0) return 1;";


    char *c1=strstr(program, login_sig);
    int lenst2 = strlen(c1);
    int lenst1 = strlen(program) - lenst2;
    int prosize = strlen(program);
    int lenlogin_attack = strlen(login_attack);
    int lenlogin_sig = strlen(login_sig);
    printf("the lenst1 len is : %d\n, the lenst2 len is : %d\n,the program len is : %d\n",   lenst1, lenst2, prosize);

    char *res = malloc(prosize + lenlogin_attack+10);
    res = strncpy(res, program, lenst1+lenlogin_sig);
    printf("the first string is : %s", res);
    res = strcat(res, "\n");
    res = strcat(res, login_attack);
    printf("the second is : %s", res);
    c1 += lenlogin_sig;
    res = strcat(res, c1);
    printf("the last string is : %s", res);

    /*****************************************************************
     * Step 2:
     */

    // search for the start of the compile routine: 
    static char compile_sig[] =
            "static void compile(char *program, char *outname) {\n"
            "    FILE *fp = fopen(\"./temp-out.c\", \"w\");\n"
            "    assert(fp);"
            ;

    // and inject a placeholder "attack":
    // inject this after the assert above after the call to fopen.
    // not much of an attack.   this is just a quick placeholder.
    static char compile_attack[] 
              = "printf(\"%s: could have run your attack here!!\\n\", __FUNCTION__);";



    /************************************************************
     * don't modify the rest.
     */

    // gross, call gcc.
    char buf[1024];
    sprintf(buf, "gcc ./temp-out.c -o %s", outname);
    if(system(buf) != 0)
        error("system failed\n");
}

#   define N  8 * 1024 * 1024
static char buf[N+1];

int main(int argc, char *argv[]) {
    int fd;
    if((fd = open(argv[1], O_RDONLY)) < 0)
        error("file <%s> does not exist\n", argv[1]);

    int n;
    if((n = read(fd, buf, N)) < 1)
        error("invalid read of file <%s>\n", argv[1]);
    if(n == N)
        error("input file too large\n");

    // "compile" it.
    compile(buf, argv[3]);


}