#include <stdio.h>
#include <stdlib.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>

void server_monitor(void)
{
        struct utsname un;
        struct sysinfo sys;

        if(sysinfo(&sys) < 0) {
                fprintf(stderr, "sysinfo() function error.\n");
                return 1;
        }
        if(uname(&un) < 0) {
                fprintf(stderr, "uname() function error.\n");
                return 1;
        }

        fprintf(stdout,
                        "============= [ 서버 자원 ] ============\n"
                        "노드명 : %s\n"
                        "시스템 이름 : %s\n"
                        "커널 버전 : %s\n"
                        "프로세서타입 : %s\n"
                        "CPU 개수 : %d\n\n"
                        "============== [ 메모리 ] ==============\n"
                        "총 메모리 : %d KB (%d GB)\n"
                        "사용가능한 메모리 : %d KB (%d GB)\n"
                        "버퍼 메모리 : %d KB (%d GB)\n"
                        "총 Swap 공간 : %d KB (%d GB)\n"
                        "사용 가능한 Swap 공간 : %d KB (%d GB)\n",
                        un.nodename, un.sysname, un.release, un.machine, get_nprocs_conf(),
                        sys.totalram/1024, sys.totalram/1024000000,
                        sys.freeram/1024, sys.freeram/1024000000,
                        sys.bufferram/1024, sys.bufferram/1024000000,
                        sys.totalswap/1024, sys.totalswap/1024000000,
                        sys.freeswap/1024, sys.freeswap/1024000000
                );
}
