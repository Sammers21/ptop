/* 
Copyright 2017 Pavel Drankov

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <locale.h>
// hash lib
#include "uthash.h"

#define RELEASE
//#define LOGS

struct process {
    int pid;
    int ppid;
    int cur_child;
    char user_name[50];
    char process_name[1000];
    struct process **child;
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
    p->ppid = ppid;
    p->cur_child = 0;
    p->child = malloc(sizeof(struct process) * 50);
    strcpy(p->user_name, user_name);
    strcpy(p->process_name, process_name);
    HASH_ADD_INT(processes, pid, p);


    struct process *to_find;
    HASH_FIND_INT(processes, &ppid, to_find);
    if (to_find != NULL) {
        to_find->child[to_find->cur_child++] = p;
    }
}

void delete_all() {
    struct process *prc, *tmp;
    HASH_ITER(hh, processes, prc, tmp) {
        HASH_DEL(processes, prc);     /* delete; users advances to next */
        free(prc->child);
        free(prc);                    /* optional- if you want to free  */
    }
}

void print_all() {
    struct process *process1, *tmp;
    HASH_ITER(hh, processes, process1, tmp) {
        printf("pid %d pname %s uname %s\n", process1->pid, process1->process_name, process1->user_name);
    }
}

void rec(int *rem, struct process *p, int tabs, int stsize) {

    int space = stsize;

    if (*rem == 0) {
        return;
    }
    (*rem) -= 1;

    for (int j = 0; j < tabs - 1; ++j) {
        if (space - 4 > 0) {
            printf("|   ");
            space -= 4;
        }
    }

    if (tabs >= 1) {
        if (space - 4 > 0) {
            printf("└───");
            space -= 4;
        }
    }
    char *c = malloc(sizeof(char *) * (space));
    strncpy(c, p->process_name, space);
    c[space] = 0;
    printf("%s", c);
    printf("\n");
    free(c);

    for (int i = 0; i < p->cur_child; i++) {
        rec(rem, p->child[i], tabs + 1, stsize);
    }
}

void get_process_list(int rows, int colums) {

    struct process *to_find;
    int i = 0;
    HASH_FIND_INT(processes, &i, to_find);
    int *rem = malloc(sizeof(int *));
    *rem = rows;
    rec(rem, to_find, -1, colums);

}


char **words(char *text) {

    char **res;

    char *str = malloc(5000);
    strcpy(str, text);

    int i = 0;
    char *pch;
    pch = strtok(str, " ");
    while (pch != NULL) {
        i++;
        pch = strtok(NULL, " ");
    }

    int g = 0;
    res = malloc(sizeof(char *) * i);
    strcpy(str, text);
    pch = strtok(str, " ");
    while (pch != NULL) {
        res[g++] = pch;
        pch = strtok(NULL, " ");
    }

    return res;
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

int main() {
    setlocale(LC_ALL, "");


    while (1) {
        delete_all();
        add_process(-1, 0, "system", "init");
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

            char str[4096];
            FILE *psout;
            psout = fdopen(fd_ps[0], "r");

            if (psout) {
                while (fgets(str, 4096, psout) != NULL) {
#ifdef LOGS
                    printf("%s", str);
#endif

                    char **splt = words(str);
                    if (strcmp(splt[0], "F") == 0)
                        continue;

#ifdef  LOGS
                    printf("ppid %s\n", splt[4]);
                    printf("pid %s\n", splt[3]);
                    printf("uname %s\n", splt[2]);
                    printf("pname %s\n", splt[14]);
#endif
                    if (splt[14][strlen(splt[14]) - 1] == '\n') {
                        splt[14][strlen(splt[14]) - 1] = 0;
                    }

                    add_process(atoi(splt[4]), atoi(splt[3]), splt[2], splt[14]);

                }
                fclose(psout);
            }

        } else {
            //fork error
            printf("cant fork");
            exit(-1);
        }
#ifdef LOGS
        print_all();
#endif
        int *hw = tsize();
        get_process_list(hw[0], hw[1]);
        sleep(1);
        printf("\033[2J");
        free(hw);

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
