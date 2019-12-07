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
                        "============= [ ���� �ڿ� ] ============\n"
                        "���� : %s\n"
                        "�ý��� �̸� : %s\n"
                        "Ŀ�� ���� : %s\n"
                        "���μ���Ÿ�� : %s\n"
                        "CPU ���� : %d\n\n"
                        "============== [ �޸� ] ==============\n"
                        "�� �޸� : %d KB (%d GB)\n"
                        "��밡���� �޸� : %d KB (%d GB)\n"
                        "���� �޸� : %d KB (%d GB)\n"
                        "�� Swap ���� : %d KB (%d GB)\n"
                        "��� ������ Swap ���� : %d KB (%d GB)\n",
                        un.nodename, un.sysname, un.release, un.machine, get_nprocs_conf(),
                        sys.totalram/1024, sys.totalram/1024000000,
                        sys.freeram/1024, sys.freeram/1024000000,
                        sys.bufferram/1024, sys.bufferram/1024000000,
                        sys.totalswap/1024, sys.totalswap/1024000000,
                        sys.freeswap/1024, sys.freeswap/1024000000
                );
}