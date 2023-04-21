#include <stdio.h>
#include <stdlib.h>
#include <string.h>        // strlen()
#include <fcntl.h>         // O_WRONLY
#include <unistd.h>        // write(), close()
#include "dpram_dev.h"
#include <time.h>
#include <sys/mman.h>
#include <rtdm/rtdm.h>
//#include <trank/rtdm/rtdm.h>
//#include <native/task.h>
#include <alchemy/task.h>


#define DEVICE_NAME     "dpram"      // RTDM에 등록된 디바이스 이름 
 
RT_TASK rt_task_desc;                // RT task descriptor (rt-task에 관한 데이터가 저장되는 변수)

/* TEST 프로그램 
 *
 * dpram에 control_data, state_data를 쓰는 테스트 프로그램 
 *   - ioctl()함수를 통하여, dpram에 접근
 *   - 정해진 구조체에 대한, 접근이 용이
 */

int main(int argc, char* argv[])
{

    printf("START TEST!\n"); fflush(stdout);

    int device;
    int ret, i, k;

    ret = mlockall(MCL_CURRENT | MCL_FUTURE);              // 메모리 swap이 일어나지 않도록 방지
    if (ret) {
        perror("ERROR : mlockall has failled");
        exit(1);
    }
    ret = rt_task_shadow(&rt_task_desc, NULL, 1, 0);       // 현재, task가(process) RT task 임을 명시
    if (ret)
    {
        fprintf(stderr, "ERROR : rt_task_shadow: %s\n", strerror(-ret));
        exit(1);
    }
//    device = rt_dev_open(DEVICE_NAME, 0);                  // Device를 open()
    device = rtdm_open(DEVICE_NAME, 0);                  // Device를 open()
    if (device < 0) {
        printf("ERROR : can't open device %s (%s)\n", DEVICE_NAME, strerror(-device));
        exit(1);
    }


    // ----------- TEST  ----------------------
    int send_count = 10;            // 테스트 횟수
    state_data sdata;               // state_data 구조체 (dpram_dev.h에서 정의)
    control_data cdata;             // control_data 구조체 (dpram_dev.h에서 정의)
         

    clock_t begin = clock();
    for(k = 0; k < send_count; ++k){
        // 매 iteration 마다, dpram에 write할 값을 증가 시킴
        for(i = 0; i < CONTROL_JOINT_NUM; ++i){
            cdata.torque[i] = 1.0*i;//i*3 + 10.123123+k;
            cdata.velocity[i] = 2.0*i;//i*3 + 11.4334+k;
            cdata.position[i] = 3.0*i;//i*3 + 12.5343+k;
            sdata.torque[i] = 4.0*i;//i*3 + 13.123123+k;
            sdata.velocity[i] = 5.0*i;//i*3 + 14.4334+k;
            sdata.position[i] = 6.0*i;//i*3 + 15.5343+k;
        }

        rt_dev_ioctl(device, DPRAM_SEND_CONTROL_DATA, &cdata);     // ioctl() 함수를 통하여, control_data를 씀
        rt_dev_ioctl(device, DPRAM_SEND_TEST_STATE_DATA, &sdata);  // ioctl() 함수를 통하여, state_data를 씀
    }

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    printf("Spent time: %.4f s\t for %.4f MByte\n", time_spent, (double)send_count*sizeof(control_data)/1024.0/1024.0);
    // ----------------------------------------


    ret = rt_dev_close(device);        // Device close()
    if (ret < 0) {
        printf("ERROR : can't close device %s (%s)\n", DEVICE_NAME, strerror(-ret));
        exit(1);
    }
    printf("=== END TEST ===\n"); fflush(stdout);
    return 0;
}
