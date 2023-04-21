#include <stdio.h>
#include <stdlib.h>
#include <string.h>        // strlen()
#include <fcntl.h>         // O_WRONLY
#include <unistd.h>        // write(), close()
#include <dpram_dev.h>
#include <time.h>
#include <sys/mman.h>
#include <rtdm/rtdm.h>
#include <native/task.h>

#define DEVICE_NAME     "dpram"      // RTDM에 등록된 디바이스 이름 
 
RT_TASK rt_task_desc;                // RT task descriptor (rt-task에 관한 데이터가 저장되는 변수)


/* TEST 프로그램 
 *
 * myCortex와 함께 전체 저장공간(0~0xfff)에 숫자를 하나씩 증가시켜가며 dpram 공유 테스트 프로그램
 *   0(mycortex)->1(bbb)->2(mycortex)->3(bbb)-> ... 
 *   - 전체 공간에 대해서, 지속적으로 read()를 하여 값이 달라진 경우에 대해서만 write() 수행
 */

int main(int argc, char* argv[])
{

    printf("START TEST!\n"); fflush(stdout);

    char buf[1024];
    ssize_t size;
    int device;
    int ret, i, j;
    const int max_count = 100000;    // 테스트 횟수
    unsigned short val = 1;          // 초기값 == 1

    unsigned short data_in[DPRAM_MAX_SIZE] = {1};    // read()를 위한 저장공간
    unsigned short data_out[DPRAM_MAX_SIZE] = {1};   // write()를 위한 저장공간



    double transfer_time = 0;                        // 누적된 전송 시간
    size_t data_size = 0;                            // 누적된 전송 데이터량


    ret = mlockall(MCL_CURRENT | MCL_FUTURE);        // 메모리 swap이 일어나지 않도록 방지
    if (ret) {
        perror("ERROR : mlockall has failled");
        exit(1);
    }

    ret = rt_task_shadow(&rt_task_desc, NULL, 1, 0);  // 현재, task가(process) RT task 임을 명시
    if (ret)
    {
        fprintf(stderr, "ERROR : rt_task_shadow: %s\n", strerror(-ret));
        exit(1);
    }

    device = rt_dev_open(DEVICE_NAME, 0);            // Device를 open()
    if (device < 0) {
        printf("ERROR : can't open device %s (%s)\n", DEVICE_NAME, strerror(-device));
        exit(1);
    }


    // 테스트 시작
    for(i = 0; i < max_count; ++i){
        clock_t begin = clock();       // 시간 측정 시작
        rt_dev_read(device, (void*)data_in, sizeof(short)*DPRAM_MAX_SIZE);               // 전체 메모리 공간을 read()
        clock_t end = clock();         // 시간 측정 종료
        transfer_time = transfer_time + (double)(end - begin)/CLOCKS_PER_SEC;            // 전송 시간 누적
        data_size += sizeof(short)*DPRAM_MAX_SIZE;                                       // 전송 데이터 누적
        if(data_in[0] != val){         // 기존에 메모리에 쓴 값과 다를 경우
            val = data_in[0] + 1;      // 값을 +1 하여 업데이트
            for(j = 0; j< DPRAM_MAX_SIZE;++j){
                data_out[j] = val;     // 업데이트 된 값을 적용
            }
            clock_t begin = clock();   // 시간 측정 종료
            rt_dev_write(device, (const void*)data_out, sizeof(short)*DPRAM_MAX_SIZE);   // 바뀐 데이터를 전체 메모리 공간에 write()
            clock_t end = clock();     // 시간 측정 종료
            transfer_time = transfer_time + (double)(end - begin)/CLOCKS_PER_SEC;        // 전송 시간 누적
            data_size += sizeof(short)*DPRAM_MAX_SIZE;                                   // 전송 데이터 누적 

        }

        int error = 0;
        for(j = 0; j < DPRAM_MAX_SIZE; ++j){
            error += abs((int)data_in[j] - (int)data_out[j]);                            // 전체 메모리 공간에 대해서, 기존의 읽은 값과 기입한 값의 차이를 계산
        }

        // 쓴 값과 읽은 값이 동일(error==0)하거나, mycortex에서 +1을 한 경우(error==DPRAM_MAX_SIZE)에 대해서는 무시.
        //   하지만, semaphore가 정상동작하지 않을 경우, 메모리 access가 동시(bbb & mycortex)에 이루어지므로 stable한 전송이 이루어지지 않는다. 해당 경우에 대해서 그 에러값이 표시됨.
        if((error != 0 && error != DPRAM_MAX_SIZE)){                                     
            printf("[%d]Error: %d, Out: 0x%X,  In : 0x%X\n",i, error, data_out[0], data_in[0]);
        }
        if(i % 1000 == 0){
            double mbs = (float)data_size/1024.0/1024.0/transfer_time;
            printf("[%d]STEP: Current transfered size: %.4f MByte, time: %.4f sec, speed: %.4f MB/s\n", i, (float)data_size/1024.0/1024.0, transfer_time, mbs);
        }
    }

    double mbs = (float)data_size/1024.0/1024.0/transfer_time;
    printf("Total transfered size: %.4f MByte, time: %.4f sec, speed: %.4f MB/s\n", (float)data_size/1024.0/1024.0, transfer_time, mbs);

    ret = rt_dev_close(device);   // close()
    if (ret < 0) {
        printf("ERROR : can't close device %s (%s)\n", DEVICE_NAME, strerror(-ret));
        exit(1);
    }
    printf("=== END TEST ===\n"); fflush(stdout);
    return 0;
}
