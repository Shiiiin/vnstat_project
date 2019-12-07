#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/utsname.h>

const char *CPUSTAT    = "/proc/stat";       // cpu 정보

// CPU 정보
typedef struct _cpuinfo
{
    FILE *fp;                 // 파일 스트림 포인터
    char cpuname[50];         //1) cpu  : cpu 구분
    long long cpu_user;       //2) user : 사용자 영역 코드 실행 시간
    long long cpu_nice;       //3) system: 커널 영역 코드 실행 시간
    long long cpu_system;     //4) nice : 기본보다 낮은 우선순위로 실행한 사용자 영역 코드 실행 시간
    long long cpu_idle;       //5) idle : 대기 시간
    long long cpu_wait;       //6) wait : I/O 완료 대기 시간
    long long cpu_hi;         //7) hi : Hard Interrupt(IRQ)
    long long cpu_si;         //8) si : Soft Interrupt(SoftIRQ)
    long long cpu_zero;       //9) zero : 끝
} CPU_P;

CPU_P *cpu_dfopen()
{
    CPU_P *CP;
    // /proc/stat 파일을 연다.
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

    // /proc/stat 으로 부터 CPU 정보를 얻어온다.
    while(fgets(c_buf, 1024, CP->fp))
    {
        sscanf(c_buf, "%s",CP->cpuname);
        if (strncmp(CP->cpuname,"cpu",3) == 0)
        {
            // CPU 시스템의 현재 사용량을 구한다.
            sscanf(c_buf, "%s %llu %llu %llu %llu %llu %llu %llu %llu"
                ,CP->cpuname, &CP->cpu_user, &CP->cpu_nice, &CP->cpu_system, &CP->cpu_idle, &CP->cpu_wait, &CP->cpu_hi, &CP->cpu_si, &CP->cpu_zero);
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

    fprintf(stdout, "CPU 개수 : %d \n", get_nprocs_conf());
    printf("=============================================== [ CPU 정보 ] ================================================ \n\n" );
    while(cpu_dfget(CP))
    {
        // 모든 정보 보기
        printf("%-10s %10llu %10llu %10llu %10llu %10llu %10llu %10llu " , CP->cpuname, CP->cpu_user, CP->cpu_nice, CP->cpu_system, CP->cpu_idle, CP->cpu_wait, CP->cpu_hi, CP->cpu_si );
        // CPU사용량 정보 보기
        printf(", Usage: %10.2f %% \n" , (float)(CP->cpu_user + CP->cpu_nice + CP->cpu_system) / (CP->cpu_user + CP->cpu_nice + CP->cpu_system + CP->cpu_idle) * 100 );
    }
}