#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// hash lib
#include "uthash.h"

#define RELEASE

struct process {
    int pid;
    char *user_name;
    char *process_name;
    struct process **child;
    int cur_child;
    UT_hash_handle hh;
};

//init hash
struct process *processes = NULL;    /* important! initialize to NULL */

char **str_split(char *a_str, const char a_delim);

void fill_screen(char **text);

void add_process(int ppid, int pid, char *user_name, char *process_name) {

    struct process *p;
    p = malloc(sizeof(struct process));
    p->pid = pid;
    p->cur_child = 0;
    strcpy(p->user_name, user_name);
    strcpy(p->process_name, process_name);
    HASH_ADD_INT(processes, pid, p);

    if (ppid != 0) {
        struct process *to_find;
        HASH_FIND_INT(processes, &ppid, to_find);
        if (to_find != NULL) {
            to_find->child[to_find->cur_child++] = p;
        }
    }
}

/**
 * @return  return array sizeof 2 with [0] - terminal rows count and [1] - terminal columns count
 */
int *tsize();
/*
printf("\033[XA"); // Move up X lines;
printf("\033[XB"); // Move down X lines;
printf("\033[XC"); // Move right X column;
printf("\033[XD"); // Move left X column;
printf("\033[2J"); // Clear screen
 */
//man console_codes

//stty -a
//ps -efl
int main() {
    while (1) {
        /**
         * fd[0] read only
         * fd[1] write only
         */
        //ps process
        int fd_ps[2];

        int p = pipe(fd_ps);

        if (p != 0) {
            printf("cant create pipe");
            exit(-1);
        }


        int result = fork();

        if (result == 0) {
            close(fd_ps[0]);
            //child process
            //redirect output

            dup2(fd_ps[1], STDOUT_FILENO);
            execl("/bin/ps", "ps", "-efl", NULL);
            exit(0);

        } else if (result > 0) {
            //parent process
            close(fd_ps[1]);

            char str[5000];
            FILE *psout;
            psout = fdopen(fd_ps[0], "r");

            if (psout) {
                while (fgets(str, 5000, psout) != NULL) {
                    printf("%s", str);
                }
                fclose(psout);
            }

        } else {
            //fork error
            printf("cant fork");
            exit(-1);
        }
        sleep(1);
    }

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

        FILE *psout = fdopen(fd[0], "r");
        char *c = fgets(foo, 4096, psout);
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
        close(fd[0]);
        close(fd[1]);

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

void fill_screen(char **text) {
    int *hw = tsize();
    //clear screen
    printf("\033[2J");
    for (int y = 1; y < hw[0]; ++y) {
        for (int x = 1; x < hw[1] + 1; ++x) {
            printf("\033[%d;%dHf%c", y, x, text[y - 1][x - 1]);
        }
    }

    free(hw);
}