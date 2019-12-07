#include <dirent.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>

#define DECIMAL (10)
#define BUFFER_SIZE (1024)

/* 좀비 프로세스를 탐색하고 해당 좀비의 부모 프로세스를 KILL 합니다 */
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

	/* "/proc" 디렉토리를 엽니다. 이 디렉토리에는 현재 실행중인 프로세스들에 대한 정보가 모두 들어있습니다. */
	/* 실패하면 NULL 을 반환하기 때문에 예외처리 했습니다. */
	directory = opendir("/proc");
	if (directory == NULL) {
		printf("directory open failed!\n");
		return -1;
	}

	printf("Searching for Zombie Processes...\n\n");

	/* 디렉토리 정보를 하나씩 읽으면서 반복합니다. */
	while ((entry = readdir(directory)) != NULL) {

		/* "/proc" 에는 숫자로 된 데이터와 문자로 된 데이터들이 혼재하는데, 숫자로 된 데이터들이 프로세스들의 pid 를 의미합니다. */
		/* 따라서 안에 들어있는 내용들 중 우리가 관심있는 pid 만 선별하기 위한 작업입니다. */
		/* strtol 함수는 string to long 즉, 문자열을 long 타입 정수로 변환해줍니다. */
		/* 그리고 변환 후 문자열의 끝 부분을 반환합니다 (endptr) */
		/* 그런데 만약 숫자를 읽었다면 ex) 11_ */
		/* endptr 이 11을 읽고 11 뒤에 있는 공간을 가리키겠죠? (이 공간에는 '\0' 이라는 빈 문자가 들어있습니다.) */
		/* 하지만 만약 문자를 읽었다면 ex) abc */
		/* 해당 문자에 숫자가 없으므로 endptr이 a에 멈춰 있습니다. */
		/* 이를 이용해 strtol 을 통해 읽어드린 정보를 long 으로 변환해보고 */
		/* 그 후 endptr 이 만약 빈 공간을 가리키고 있지 않으면, 숫자가 아닌것으로 판단해 continue 합니다. */
		pid = strtol(entry->d_name, &endptr, DECIMAL);
		if (*endptr != '\0') {
			continue;
		}

		/* pid 를 얻었으면 다음 할 일은 해당 프로세스의 세부 정보를 보기 위해 "/proc/[pid]/stat" 파일을 읽는 것입니다. */
		/* 버퍼에 문자열을 조합해 저장하고 해당 버퍼를 통해 파일을 읽습니다. */
		/* 실패하면 NULL 을 반환하기 때문에 예외처리 했습니다. */
		sprintf(buffer, "/proc/%ld/stat", pid);
		file = fopen(buffer, "r");
		if (file == NULL) {
			printf("stat file open failed!\n");
			continue;
		}

		/* stat 파일에 들어있는 내용들을 읽을 것입니다. 일단 모두 읽어서 buffer 에 저장합니다. */
		if (fgets(buffer, sizeof(buffer), file) == NULL) {
			printf("stat file read failed!\n");
			fclose(file);
			continue;
		}

		/* stat 파일에 들어 있는 세부 내용은 해당 블로그를 참조하시옵소서. */
		/* https://hoon038.tistory.com/10 */
		/* 보시면 아시겠지만 각 정보들이 공백 " " 으로 구분되어 쭉 나열되어 있습니다. */
		/* 그래서 strtok 함수를 이용해 buffer 에 읽어드린 내용을 공백 단위로 쪼개겠습니다. */
		/* 그러면 pid, name, state, ppid 순서대로 쪼개지겠죠? 각각 변수에 저장합니다. */
		process_pid = strtok(buffer, " ");
		process_name = strtok(NULL, " ");
		process_state = strtok(NULL, " ");
		process_ppid = strtok(NULL, " ");

		/* 그리고 state 가 Z인 좀비 프로세스들을 찾아냅니다. */
		if (!strcmp(process_state, "Z")) {
			printf("%s is Zombie\n", process_name);
			++zombie_count;

			/* 찾아낸 좀비의 부모 프로세스에게 SIGNAL 을 보냅니다. 여기서는 KILL 시그널을 보냈어요. (9번 시그널) */
			/* 즉 좀비의 부모를 죽임으로서 그 자식인 좀비까지 죽입니다. */
			if (!kill(atoi(process_ppid), SIGKILL)) {
				printf("K I L L !!\n");
			}
		}

		fclose(file);
	}

	closedir(directory);

	printf("%d Zombie Processes were detected... !!\n\n", zombie_count);

	return 0;
}
