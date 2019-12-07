#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define DECIMAL (10)
#define BUFFER_SIZE (1024)

/* 프로세스 정보를 수집해 좀비 프로세스 수를 파악합니다. */
void zombie_num(void)
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

	printf("Searching for Zombie Processes...\n\n");

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
                        ++zombie_count;
                }

                fclose(file);
        }

        closedir(directory);

        printf("%d Zombie Processes were detected... !!\n\n", zombie_count);

        return 0;
}

