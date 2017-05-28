#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


char **str_split(char *a_str, const char a_delim);

/**
 * @return  return array sizeof 2 with [0] - terminal rows count and [1] - terminal columns count
 */
int *tsize();

//stty -a
//ps -efl
int main() {

    int *hw = tsize();

    printf("coloumns %d", hw[0]);
    printf("rows %d", hw[1]);

    free(hw);
#ifdef Debug
    while (0) {

        /**
         * fd[0] read only
         * fd[1] write only
         */
        //ps process
        int fd_ps[2];

        int p = pipe(fd_ps);
        char foo[4096];

        //stty process
        int fd_stty[2];
        int p2 = pipe(fd_stty);

        if (p != 0 || p2 == 0) {

            printf("cant create pipe");
            exit(-1);
        }


        int result = fork();


        if (result == 0) {

            //child process
            printf("hello from child process\n");

            //redirect output
            dup2(fd_ps[1], STDOUT_FILENO);

            close(fd_ps[0]);
            close(fd_ps[1]);

            execl("/bin/ps", "ps", "-efl", NULL);
            exit(0);

        } else if (result > 0) {
            //parent process
            printf("hello from parent process\n");
            close(fd_ps[1]);

            FILE *psout = fdopen(fd_ps[0], "r");
            char *s;
            do {
                s = fgets(foo, 4096, psout);
                printf("%s", s);
            } while (s != NULL);


        } else {
            //fork error
            printf("cant fork");
            exit(-1);
        }
        sleep(1);
    }
#endif

    return 0;
}

int *tsize() {

    /**
     * fd[0] read only
     * fd[1] write only
     */
    //ps process
    int fd[2];

    int p = pipe(fd);
    char foo[4096];

    if (p != 0) {

        printf("cant create pipe");
        exit(-1);
    }


    int result = fork();


    if (result == 0) {


        //redirect output
        dup2(fd[1], STDOUT_FILENO);

        close(fd[0]);
        close(fd[1]);

        execl("/bin/stty", "stty", "-a", NULL);
        exit(0);

    } else if (result > 0) {
        //parent process
        close(fd[1]);

        FILE *psout = fdopen(fd[0], "r");
        close(fd[1]);
        char *c = fgets(foo, 4096, psout);

        printf("%s", c);
        printf("%s", c);
        printf("%s", c);


        char **splt = str_split(c, ' ');

        //delete last char ';'
        splt[4][strlen(splt[4]) - 1] = 0;
        splt[6][strlen(splt[6]) - 1] = 0;

        int h = atoi(splt[4]);
        int w = atoi(splt[6]);

        int *pointer = malloc(sizeof(int *) * 2);
        pointer[0] = h;
        pointer[1] = w;

        free(splt);
        return pointer;


    } else {
        //fork error
        printf("cant fork");
        exit(-1);
    }
}

char **str_split(char *a_str, const char a_delim) {
    char **result = 0;
    size_t count = 0;
    char *tmp = a_str;
    char *last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char *) * count);

    if (result) {
        size_t idx = 0;
        char *token = strtok(a_str, delim);

        while (token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}