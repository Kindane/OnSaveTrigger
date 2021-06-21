#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <libgen.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

jmp_buf ebuf;             /* temp buffer for longjmp */
static int terminate = 0; /* this variable needs for SIGINT */

void exit_program(int);
int dir_is_exists(const char* path);

int main(int argc, char **argv) {

    if (argc < 3 || argc > 3) {
        printf("Usage: %s <FULL_PATH_TO_FILE> <COMMAND>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, exit_program); // If ^C was pressed: exit program

    const char* command = argv[2];
    const char* file_name = basename(argv[1]);
    const int size_of_path = strlen(argv[1]) - strlen(file_name);
    char* path = calloc(size_of_path, sizeof(char));
    strncpy(path, argv[1], size_of_path - 1); // copy path without filename to path

    if (!dir_is_exists(path)) {
        printf("Error. Directory does not exists\n");
        exit(EXIT_FAILURE);
    }

    int length;
    int fd;               // file descriptor
    int wd;               // watch descriptor
    char buffer[BUF_LEN]; // temporary buffer

    fd = inotify_init();

    if (fd < 0) {
        perror("inotify_init");
        exit(EXIT_FAILURE);
    }

    wd = inotify_add_watch(fd, path, IN_MODIFY);
    setjmp(ebuf);
    int i = 0;
    // main loop
    while (1) {
        if (terminate) break;
        length = read(fd, buffer, BUF_LEN);
        i = 0;

        if (length < 0)
            perror("read");

        while (i < length) {
            struct inotify_event* event = (struct inotify_event*) &buffer[i]; // send info from temp buffer to event

            if (event->len) // check for event
                if (event->mask & IN_MODIFY) // if event is modify event
                    if (!(event->mask & IN_ISDIR)) // it is not a directory
                        if (!strcmp(file_name, event->name)) // if filename == event.name
                            system(command);
            i += EVENT_SIZE + event->len;
        }
    }
    inotify_rm_watch(fd, wd);
    close(fd);
    free(path);
    return 0;
}

int dir_is_exists(const char *path) {
    DIR *dir = opendir(path);
    if (dir) {
        /* Directory exists. */
        closedir(dir);
        return 1;
    }
    return 0;
}

void exit_program(int value) {
    printf("\nExiting...\n");
    terminate = 1;
    longjmp(ebuf, 0);
}