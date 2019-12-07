#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/utsname.h>

const char *CPUSTAT    = "/proc/stat";

typedef struct _cpuinfo
{
    FILE *fp;
    char cpuname[50];
    long long cpu_user;	
    long long cpu_nice;	
    long long cpu_system;
    long long cpu_idle;
    long long cpu_wait;	
    long long cpu_hard_interrupt;
    long long cpu_soft_interrupt;
    long long cpu_zero;
} CPU_P;

CPU_P *cpu_dfopen()
{
    CPU_P *CP;
    CP = (CPU_P *)malloc(sizeof(CPU_P));
    if(!(CP->fp = fopen(CPUSTAT, "r")) )
    {
        return NULL;
    }
    else
        return CP;
}

CPU_P *cpu_dfget(CPU_P *CP)
{
    char c_buf[1024];

    while(fgets(c_buf, 1024, CP->fp))
    {
        sscanf(c_buf, "%s",CP->cpuname);
        if (strncmp(CP->cpuname,"cpu",3) == 0)
        {
            sscanf(c_buf, "%s %llu %llu %llu %llu %llu %llu %llu %llu"
                ,CP->cpuname, &CP->cpu_user, &CP->cpu_nice, &CP->cpu_system, &CP->cpu_idle, &CP->cpu_wait, &CP->cpu_hard_interrupt, &CP->cpu_soft_interrupt, &CP->cpu_zero);
            return CP;
        }
    }
    rewind(CP->fp);
    return NULL;
}

int dfclose(CPU_P *CP)
{
    fclose(CP->fp);
}

void cpu_info(void)
{
    struct utsname un;
    CPU_P *CP;
    char pbuf[1024];

    if(uname(&un) < 0) {
            fprintf(stderr, "uname() error.\n");
            return 1;
    }

    if ((CP=cpu_dfopen()) == NULL)
    {
        perror("error");
        return 1;
    }

    fprintf(stdout, "총 CPU 개수  : %d \n", get_nprocs_conf());
    printf("=============================================== [ CPU 정보 ] ================================================ \n\n" );
    while(cpu_dfget(CP))
    {
        printf("%-10s %10llu %10llu %10llu %10llu %10llu %10llu %10llu " , CP->cpuname, CP->cpu_user, CP->cpu_nice, CP->cpu_system, CP->cpu_idle, CP->cpu_wait, CP->cpu_hard_interrupt, CP->cpu_soft_interrupt );
        printf(", Usage: %10.2f %% \n" , (float)(CP->cpu_user + CP->cpu_nice + CP->cpu_system) / (CP->cpu_user + CP->cpu_nice + CP->cpu_system + CP->cpu_idle) * 100 );
    }
}
