#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <unistd.h>

#define EVENT_SIZE  (sizeof(struct inotify_event))
#define BUF_LEN     (1024 * (EVENT_SIZE + 16))

void get_file_name(char* restrict, char* restrict);

int main(int argc, char **argv) {

    if (argc < 3) {
        printf("ERROR! Not enough arguments.\n");
        printf("Usage: %s FILE \"COMMAND\"\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char* file_name = (char*)malloc(32*sizeof(char*));
    char* path = (char*)malloc(32*sizeof(char*));
    char* command = (char*)malloc(128*sizeof(char*));
    strcpy(command, argv[2]); // argv[2] is a command, but i name it "command"

    get_file_name(argv[1], file_name); // argv[1] is a filename

    strncpy(path, argv[1], strlen(argv[1]) - strlen(file_name) - 1); // copy path without filename to path


    int length, i = 0;
    int fd; // file descriptor
    int wd; // watch descriptor
    char buffer[BUF_LEN]; // temporary buffer

    fd = inotify_init();

    if (fd < 0)
        perror("inotify_init");

    wd = inotify_add_watch(fd, path, IN_MODIFY);
    while (1) {
        length = read(fd, buffer, BUF_LEN);
        i = 0; // TODO fix bug.

        if (length < 0)
            perror("read");

        while (i < length) {
            struct inotify_event *event = (struct inotify_event *) &buffer[i]; // send info from temp buffer to event

            if (event->len) { // check for event
                if (event->mask & IN_MODIFY) { // if event is modify event
                    if (!strcmp(file_name, event->name)) { // if filename == event.name
                        system(command);
                    }
                }
            }
            i += EVENT_SIZE + event->len;

        }
    }

    (void) inotify_rm_watch(fd, wd);
    (void) close(fd);
    free(file_name);
    free(path);
    free(command);

    return 0;

}


/*
 * INPUT PARAMS:
 * path - argv[1];
 * buffer - variable that will be contain a filename
 * */
void get_file_name(char* restrict path, char* restrict buffer) {
    char* path_copy = (char*)malloc(64*sizeof(char*)); // i need this because strtok is splits a original string
    char* precious_name = (char*)malloc(64*sizeof(char*)); // temp variable
    char* result = (char*)malloc(64*sizeof(char*)); // result

    strcpy(path_copy, path); // copy path to path_copy

    precious_name = strtok(path_copy, "/"); // split path_copy by "/"

    while (precious_name != NULL) {
        strcpy(result, precious_name);
        precious_name =  strtok(NULL, "/"); // IDK why null :D, but this is works good
    }

    strcpy(buffer, result); // return result
    // Free space
    free(precious_name);
    free(result);
    free(path_copy);
}

