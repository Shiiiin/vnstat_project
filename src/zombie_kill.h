#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define DECIMAL (10)
#define BUFFER_SIZE (1024)

void zombie_kill(void)
{
        DIR* directory;
        struct dirent* entry;
        char* endptr;
        long pid;

        char buffer[BUFFER_SIZE];
        FILE* file;

        char* process_pid;
        char* process_name;
        char* process_state;
        char* process_ppid;

        size_t zombie_count = 0;

        directory = opendir("/proc");
        if (directory == NULL) {
                printf("directory open failed!\n");
                return -1;
        }

        while ((entry = readdir(directory)) != NULL) {
                pid = strtol(entry->d_name, &endptr, DECIMAL);
                if (*endptr != '\0') {
                        continue;
                }

                sprintf(buffer, "/proc/%ld/stat", pid);
                file = fopen(buffer, "r");
                if (file == NULL) {
                        printf("stat file open failed!\n");
                        continue;
                }

                if (fgets(buffer, sizeof(buffer), file) == NULL) {
                        printf("stat file read failed!\n");
                        fclose(file);
                        continue;
                }

                process_pid = strtok(buffer, " ");
                process_name = strtok(NULL, " ");
                process_state = strtok(NULL, " ");
                process_ppid = strtok(NULL, " ");

                if (!strcmp(process_state, "Z")) {
                        printf("%s is Zombie\n", process_name);
                        ++zombie_count;

                        if (!kill(atoi(process_ppid), SIGCHLD)) {
                                printf("K I L L !!\n");
                        }
                }

                fclose(file);
        }

        closedir(directory);

        printf("\n%d zombies detected... !!\n", zombie_count);

        return 0;
}

