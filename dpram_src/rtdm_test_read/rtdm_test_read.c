#include <stdio.h>
#include <stdlib.h>
#include <string.h>        // strlen()
#include <fcntl.h>         // O_WRONLY
#include <unistd.h>        // write(), close()
#include "dpram_dev.h"
#include <time.h>
#include <sys/mman.h>
#include <rtdm/rtdm.h>
#include <trank/rtdm/rtdm.h>
//#include <native/task.h>
#include <alchemy/task.h>

#include "dpram_dev.h"

#define DEVICE_NAME     "dpram"      // RTDM에 등록된 디바이스 이름 
 
RT_TASK rt_read_desc;                // RT task descriptor (rt-task에 관한 데이터가 저장되는 변수)

/* TEST 프로그램 
 *
 * dpram에 저장된 control_data, state_data를 읽는 테스트 프로그램 
 *   - ioctl()함수를 통하여, dpram에 접근
 *   - 정해진 구조체에 대한, 접근이 용이
 */

void read_task()
{
    int ret, i, k;
    int device;
 // ----------- TEST  ----------------------
    state_data sdata;               // state_data 구조체 (dpram_dev.h에서 정의)
    control_data cdata;             // control_data 구조체 (dpram_dev.h에서 정의)

    int send_count = 100;         // 테스트 횟수

    device = rt_dev_open(DEVICE_NAME, 0);                       // Device를 open()

    if (device < 0) {
        printf("ERROR : can't open device %s (%s)\n", DEVICE_NAME, strerror(-device));
        exit(1);
    }


    clock_t begin = clock();
    for(k = 0; k < send_count; ++k){
        rt_dev_ioctl(device, DPRAM_RECV_TEST_CONTROL_DATA, &cdata);    // ioctl() 함수를 통하여, control_data를 읽음
        rt_dev_ioctl(device, DPRAM_RECV_STATE_DATA, &sdata);           // ioctl() 함수를 통하여, state_data를 읽음
    }
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    double mbs = (double)send_count*(sizeof(state_data)+sizeof(control_data))/1024.0/1024.0/time_spent;
    printf("Spent time: %.4f s\t for %.4f MByte [%.4f MB/s]\n", time_spent, (double)send_count*(sizeof(state_data)+sizeof(control_data))/1024.0/1024.0, mbs);

    for(i = 0; i < CONTROL_JOINT_NUM; ++i){
        printf("Recv control: %.4f %.4f %.4f \n", cdata.torque[i], cdata.velocity[i], cdata.position[i]);    // 읽은 control_data를 표시
    }
    printf("=========================================\n");
    for(i = 0; i < CONTROL_JOINT_NUM; ++i){
        printf("Recv state: %.4f %.4f %.4f \n", sdata.torque[i], sdata.velocity[i], sdata.position[i]);      // 읽은 state_data를 표시
    }

    // ----------------------------------------


    ret = rt_dev_close(device);     // Device close()
    if (ret < 0) {
        printf("ERROR : can't close device %s (%s)\n", DEVICE_NAME, strerror(-ret));
        exit(1);
    }
}

int main(int argc, char* argv[])
{

    printf("START TEST!\n"); fflush(stdout);

    int ret, i, k;
    char str[30];

    ret = mlockall(MCL_CURRENT | MCL_FUTURE);              // 메모리 swap이 일어나지 않도록 방지
    if (ret) {
        perror("ERROR : mlockall has failled");
        exit(1);
    }

    sprintf(str, "rtdm_test_read_task");

    rt_task_create(&rt_read_desc, str, 0, 50, T_JOINABLE);

    rt_task_start(&rt_read_desc, &read_task,0);

    rt_task_join(&rt_read_desc);

   
    printf("=== END TEST ===\n"); fflush(stdout);
    return 0;
}
