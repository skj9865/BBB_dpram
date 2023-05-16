#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/time.h>
#include <asm/uaccess.h>   /* copy_to_user */
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/memory.h>
#include <linux/mutex.h>
#include <linux/gpio.h>

//device tree support
#include <linux/of.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/completion.h>

#include <rtdm/driver.h>

#include "dpram_dev.h"             // ioctl instructions 정의에 관한 헤더파일 

#define DEVICE_NAME "dpram"        // 생성할 디바이스 파일 이름 정의


#define SEM_DATA_LOCK   (0x0)      // semaphore에 0을 기입시 lock
#define SEM_DATA_UNLOCK (0x1)      // semaphore에 1을 기입시 unlock

#define DPRAM_SUB_CLASS  (0x100)   // RTDM에서 디바이스를 구분하기 위한 정보


/******* DPRAM ADDRESS USAGE (normal) *******
 *                                          *
 * 0x0 ------------------------------------ *
 *             control_data area            *
 * ---------------------------------------- *
 *               state_data area            *
 * ---------------------------------------- *
 *                  empty                   *
 * 0xfff ---------------------------------- *
 *                                          *
 ********************************************/

/****** DPRAM ADDRESS USAGE (semaphore) *****
 *                                          *
 * 0x0 ------------------------------------ *
 *             control_data semaphore       *
 * 0x1 ------------------------------------ *
 *               state_data semaphore       *
 * 0x2 ------------------------------------ *
 *                  empty                   *
 * 0x7 ------------------------------------ *
 *                                          *
 ********************************************/


// 디바이스 파일 operation functions 사전 정의
/* // for xenomai 2.0
static int dpram_rtdm_open(struct rtdm_dev_context *context, struct rtdm_fd * fd, int oflags); 
static int dpram_rtdm_close(struct rtdm_dev_context *context, struct rtdm_fd * fd);
static ssize_t dpram_rtdm_read_rt(struct rtdm_dev_context *context, struct rtdm_fd * fd, void *buf, size_t nbyte);
static ssize_t dpram_rtdm_write_rt(struct rtdm_dev_context *context, struct rtdm_fd * fd, const void *buf, size_t nbyte);
static ssize_t dpram_rtdm_ioctl_rt(struct rtdm_dev_context *context, struct rtdm_fd * fd, unsigned int request, void* arg);
*/
static int dpram_rtdm_open(struct rtdm_fd * fd, int oflags); 
static void dpram_rtdm_close(struct rtdm_fd * fd);
static ssize_t dpram_rtdm_read_rt(struct rtdm_fd * fd, void __user *buf, size_t nbyte);
static ssize_t dpram_rtdm_write_rt(struct rtdm_fd * fd, const void __user *buf, size_t nbyte);
static ssize_t dpram_rtdm_ioctl_rt(struct rtdm_fd * fd, unsigned int request, void __user *arg);

// DPRAM의 semaphore 저장공간(주소: 0x0~0x7)에 읽기/쓰기를 위한 함수 정의
static unsigned short dpram_read_sem(unsigned int offset); 
static unsigned short dpram_write_sem(unsigned int offset, unsigned short data);

// DPRAM의 저장공간(주소: 0x0~0xfff)에 읽기/쓰기를 위한 함수 정의
static ssize_t dpram_write_impl(struct rtdm_fd * fd, const unsigned char* buf, size_t count, size_t offset);
static ssize_t dpram_read_impl(struct rtdm_fd * fd, unsigned char* buf, size_t count, size_t offset);

// RTDM 디바이스 관련 structure 정의
static struct rtdm_driver dpram_driver = {
    .profile_info = RTDM_PROFILE_INFO(dpram, RTDM_CLASS_EXPERIMENTAL, DPRAM_SUB_CLASS, 1),
    .device_flags = RTDM_NAMED_DEVICE,
    .device_count = 2,
    .context_size = 0,

    .ops = {
	.open = dpram_rtdm_open,
	.close = dpram_rtdm_close,
        .read_rt   = dpram_rtdm_read_rt,
        .read_nrt   = dpram_rtdm_read_rt,
        .write_rt  = dpram_rtdm_write_rt,
        .write_nrt  = dpram_rtdm_write_rt,
        .ioctl_rt  = dpram_rtdm_ioctl_rt,
        .ioctl_nrt  = dpram_rtdm_ioctl_rt,
    },

};

static struct rtdm_device device[2] = {
	[0 ... 1] = {
		.driver= &dpram_driver,
		.label = "dpram",
	},
};
////////////////////////// old version (xenomai 2.0)
/*
static struct rtdm_device device = {
    .struct_version = RTDM_DEVICE_STRUCT_VER,

    .device_flags = RTDM_NAMED_DEVICE,
    .context_size = 0,
    .device_name = DEVICE_NAME,

    // 디바이스파일 open시 호출될 함수 정의
    .open_nrt = dpram_rtdm_open,

    // 그 외 디바이스파일에 관한 operation functions 정의
    .ops = {
        .close_nrt = dpram_rtdm_close,
        .read_rt   = dpram_rtdm_read_rt,
        .read_nrt   = dpram_rtdm_read_rt,
        .write_rt  = dpram_rtdm_write_rt,
        .write_nrt  = dpram_rtdm_write_rt,
        .ioctl_rt  = dpram_rtdm_ioctl_rt,
        .ioctl_nrt  = dpram_rtdm_ioctl_rt,
    },

    // 해당 디바이스를 다른 RTDM 모듈과 구분짓기 위한 metadata
    .device_class = RTDM_CLASS_EXPERIMENTAL,
    .device_sub_class = DPRAM_SUB_CLASS,
    .profile_version = 1,
    .driver_name = DEVICE_NAME,      // 디바이스파일명 (RTDM 모듈이므로, 실제 /dev 에 나타나지는 않음)
    .driver_version = RTDM_DRIVER_VER(0, 1, 2),
    .peripheral_name = "Dual-port RAM driver",
    .provider_name = "Deok-Hwa Kim",
    .proc_name = device.device_name,

};
*/
////////////////////////// old version (xenomai 2.0)

// For DPRAM specification
static const unsigned char*   dpram_base_addr = (unsigned char *)0x08000000;   // DPRAM의 물리적주소를 담는 변수 (DTS파일의 reg 변수를 통해 사전 정의됨)
static unsigned char*         dpram_virt_addr = 0;                             // DPRAM의 가상주소를 담는 변수
static const unsigned long    dpram_size = DPRAM_MAX_SIZE;                     // DPRAM의 전체 사이즈
static const unsigned int     dpram_sem_offset_control_data = 0;               // DPRAM의 control_data 접근을 위한 semaphore 주소
static const unsigned int     dpram_sem_offset_state_data = 1;                 // DPRAM의 state_data 접근을 위한 semaphore 주소

// For driver operation
static int    is_opened = 0;       // 디바이스파일이 사전에 열려져 있는지 알려주는 변수
static int    write_count = 0;     // 디버깅용 write 횟수 체크 변수


// For GPIO 
//static unsigned int       dpram_gpio_pin_size = 8;   // 디버깅용 LED GPIO pin size
static unsigned int       dpram_gpio_pin_size = 6;      // 디버깅용 LED GPIO pin size by skj
//static unsigned int       dpram_gpio_pin_numbers[] = {4, 5, 13, 12, 113, 112, 111, 110};                // 디버깅용 LED GPIO pin number (original)
//static unsigned int       dpram_gpio_pin_numbers[] = {5, 4, 14, 115, 113, 112, 111, 110};                // 디버깅용 LED GPIO pin number // by skj
static unsigned int       dpram_gpio_pin_numbers[] = {5, 4, 113, 112, 111, 110};                // 디버깅용 LED GPIO pin number // by skj

//static bool               dpram_gpio_turn_on[] = {false, false, false, false, false, false, false, false};  // 디버깅용 LED values
static bool               dpram_gpio_turn_on[] = {false, false, false, false, false, false};  // 디버깅용 LED values by skj

static unsigned int       dpram_gpio_sem_pin_number = 31;    // DPRAM의 semaphore 저장공간 접근을 위한 gpio 핀 번호
static unsigned int       dpram_gpio_cen_pin_number = 48;    // DPRAM의 chip enable을 위한 gpio 핀 번호


// semaphore lock (offset = semaphore 주소)
void lock(const unsigned int offset){
    uint16_t sem_data;

    // semaphore lock 요청
    sem_data = dpram_write_sem(offset, SEM_DATA_LOCK);
    while((sem_data & 0x1) != 0x0)
    {
    sem_data = dpram_write_sem(offset, SEM_DATA_LOCK);
        rtdm_task_sleep(1000*10);            // 10us sleep
    }
}

// semaphore unlock (offset = semaphore 주소)
void unlock(const unsigned int offset){
    uint16_t sem_data;

    // semaphore unlock 요청
    sem_data = dpram_write_sem(offset, SEM_DATA_UNLOCK);
    while((sem_data & 0x1 ) != 0x1)
    {
    sem_data = dpram_write_sem(offset, SEM_DATA_UNLOCK);
        rtdm_task_sleep(1000*10);            // 10us sleep
    }
}

// 디버깅용 LED를 통한 8bits visuallization 함수
static void gpio_display(unsigned char val){
    int i = 0;
    
    /*
    for(i =0; i<dpram_gpio_pin_size; i++){
        gpio_set_value(dpram_gpio_pin_numbers[i],0);
        rtdm_task_sleep(1000*1000*50);            // 50ms sleep
    }
    rtdm_task_sleep(1000*1000*500);            // 500ms sleep
    
    for(i =0; i<dpram_gpio_pin_size; i++){
        gpio_set_value(dpram_gpio_pin_numbers[i],1);    
        rtdm_task_sleep(1000*1000*50);            // 50ms sleep    
    }
    */
    for(i = 0; i < dpram_gpio_pin_size; ++i){
        dpram_gpio_turn_on[i] = (((val>>i)&0x1)&&0x1);
        gpio_set_value(dpram_gpio_pin_numbers[i], dpram_gpio_turn_on[i]);   // GPIO set value
    }
}


// DPRAM semaphore 저장공간의 데이터를 쓰는 함수 
static unsigned short dpram_write_sem(unsigned int offset, unsigned short data){
    unsigned short sem_val = 0x0;


    gpio_set_value(dpram_gpio_cen_pin_number, true);                 // Chip access disable (false: disable, true: enable)
    gpio_set_value(dpram_gpio_sem_pin_number, false);                // Semaphore access enable (false: disable, true: enable)
    iowrite16(data, dpram_virt_addr+offset*2);                       // Directly write data for semaphore    
    sem_val = (unsigned short)ioread16(dpram_virt_addr+offset*2);    // Directly read data for semaphore
    gpio_set_value(dpram_gpio_sem_pin_number, true);                 // semaphore access disable (false: disable, true: enable)

    return sem_val;  // return semaphore value
}

// DPRAM 일반 저장공간의 데이터를 쓰는 함수 
static ssize_t dpram_write_impl(struct rtdm_fd * fd, const unsigned char* buf, size_t count, size_t offset){

    unsigned int i = 0;
    unsigned char* temp_buf = (unsigned char*)rtdm_malloc(count);
    int err;

    // DPRAM chip access enable (false: disable, true: enable)    
    gpio_set_value(dpram_gpio_sem_pin_number, true);                // Semaphore access enable (false: disable, true: enable)
    gpio_set_value(dpram_gpio_cen_pin_number, false);

   
    if((err = rtdm_safe_copy_from_user(fd, temp_buf, buf, count)) < 0){
        rtdm_printk("copy from user error\n");
        return err;
    };

    for(i = 0; i < count/2; ++i){        
        //iowrite16(*(((unsigned short*)&buf[i*2])), (&dpram_virt_addr[offset])+i*2); // original code commented by skj
        iowrite16(*(((unsigned short*)&temp_buf[i*2])), (&dpram_virt_addr[offset])+i*2); // by skj
    }
    rtdm_free(temp_buf);

    // DPRAM chip access disable (false: disable, true: enable)
    gpio_set_value(dpram_gpio_cen_pin_number, true);

    // 디버깅용 LED setting
    gpio_display((unsigned char)++write_count);

    return 0;
}


// DPRAM 일반 저장공간의 데이터를 읽는 함수 
static ssize_t dpram_read_impl(struct rtdm_fd * fd, unsigned char* buf, size_t count, size_t offset){

    int i = 0;
    unsigned char* temp_buf = (unsigned char*)rtdm_malloc(count);
    
    // DPRAM chip access enable (false: disable, true: enable)
    gpio_set_value(dpram_gpio_sem_pin_number, true);                // Semaphore access enable (false: disable, true: enable)
    gpio_set_value(dpram_gpio_cen_pin_number, false);
    

    //rtdm_safe_copy_to_user(user_info, &buf[0], (const void*)&dpram_virt_addr[offset], count); 

    for(i = 0; i < count/2; ++i){
        *(((unsigned short*)&temp_buf[i*2])) = ioread16((&dpram_virt_addr[offset])+i*2);

    }
    rtdm_safe_copy_to_user(fd, &buf[0], temp_buf, count); 
    rtdm_free(temp_buf);

    // DPRAM chip access disable (false: disable, true: enable)
    gpio_set_value(dpram_gpio_cen_pin_number, true);

    // 디버깅용 LED setting
    gpio_display((unsigned char)--write_count);

    return 0;
}


// ioctl() - control_data를 전송하기 위한 함수
static int send_control_data(struct rtdm_fd * fd, const control_data* control_data_val){
    int transferred=0;
    
    lock(dpram_sem_offset_control_data);    // lock semaphore to access data area    
    // Write control data
    transferred = (int)dpram_write_impl(fd, (const unsigned char*)control_data_val, sizeof(control_data), 0); 
    unlock(dpram_sem_offset_control_data);  // unlock semaphore to access data area
    return transferred;
}

// ioctl() - control_data를 수신하기 위한 함수
static int recv_control_data(struct rtdm_fd * fd, control_data* control_data_val){
    int transferred;
    lock(dpram_sem_offset_control_data);    // lock semaphore to access data area
    // Read control data
    transferred = (int)dpram_read_impl(fd, (unsigned char*)control_data_val, sizeof(control_data), 0); 
    unlock(dpram_sem_offset_control_data);  // unlock semaphore to access data area
    return transferred;
}

// ioctl() - state_data를 전송하기 위한 함수
static int send_state_data(struct rtdm_fd * fd, const state_data* state_data_val){
    int transferred;
    lock(dpram_sem_offset_state_data);    // lock semaphore to access data area
    // Write state data (control_data 영역 다음에 접근)
    transferred = (int)dpram_write_impl(fd, (const unsigned char*)state_data_val, sizeof(state_data), sizeof(control_data)); 
    unlock(dpram_sem_offset_state_data);  // unlock semaphore to access data area
    return transferred;
}

// ioctl() - state_data를 수신하기 위한 함수
static int recv_state_data(struct rtdm_fd * fd, state_data* state_data_val){
    int transferred;
    lock(dpram_sem_offset_state_data);    // lock semaphore to access data area
    // Read state data (control_data 영역 다음에 접근)
    transferred = (int)dpram_read_impl(fd, (unsigned char*)state_data_val, sizeof(state_data), sizeof(control_data)); 
    unlock(dpram_sem_offset_state_data);  // unlock semaphore to access data area
    return transferred;
}


// read() - 디바이스파일을 읽을 경우 호출 (control_data를 위한 semaphore를 사용)
static ssize_t dpram_rtdm_read_rt( struct rtdm_fd * fd, void __user *buf, size_t nbyte)
{
    ssize_t transferred;
    lock(dpram_sem_offset_control_data);    // lock semaphore to access data area
    // read 수행
    transferred = dpram_read_impl(fd, (unsigned char*)buf, nbyte, 0);
    unlock(dpram_sem_offset_control_data);  // unlock semaphore to access data area
    return transferred;
}

// write() - 디바이스파일에 데이터를 쓸 경우 호출 (control_data를 위한 semaphore를 사용)
static ssize_t dpram_rtdm_write_rt(struct rtdm_fd * fd, const void __user *buf, size_t nbyte)
{
    ssize_t transferred;
    lock(dpram_sem_offset_control_data);    // lock semaphore to access data area
    // write 수행
    transferred = dpram_write_impl(fd, (const unsigned char*)buf, nbyte, 0);
    unlock(dpram_sem_offset_control_data);  // unlock semaphore to access data area
    return transferred;
}

// ioctl() - 디바이스파일에 ioctl 함수를 쓸 경우 호출
static ssize_t dpram_rtdm_ioctl_rt(struct rtdm_fd * fd, unsigned int request, void __user *arg){

    // dpram_dev.h 에 정의된 instructions에 관하여 switch 문으로 구분
    switch( request ){
        case DPRAM_SEND_CONTROL_DATA:           // control_data를 전송시
            {
                send_control_data(fd, (const control_data*)arg);
                return 0;
            }
        case DPRAM_RECV_STATE_DATA:             // state_data를 수신시
            {
                recv_state_data(fd, (state_data*)arg);
                return 0;
            }
        case DPRAM_RECV_TEST_CONTROL_DATA:      // control_data를 수신시
            {
                recv_control_data(fd, (control_data*)arg);
                return 0;
            }
        case DPRAM_SEND_TEST_STATE_DATA:        // state_data를 전송시
            {
                send_state_data(fd, (const state_data*)arg);
                return 0;
            }
    }
    return -ENOTTY;
}

// open() - 디바이스파일을 열었을 경우에 호출
static int dpram_rtdm_open(struct rtdm_fd * fd, int oflags)
{
    rtdm_printk("dpram rtdm open\n");
    // 이미 열렸을 경우에는 추가적인 설정을 안함
    if(is_opened == 1){
        rtdm_printk("%s: module already opened\n", DEVICE_NAME);
        return 0;
    }

    // 물리적 주소에 대한 접근을 커널에 요청
    request_mem_region(((unsigned long)dpram_base_addr), dpram_size, DEVICE_NAME);

    // 접근을 위한 가상 주소 할당 요청
    dpram_virt_addr = ioremap_nocache(((unsigned long)dpram_base_addr), dpram_size);

    if (dpram_virt_addr == NULL){
        rtdm_printk("ioremap returns NULL ! \n");
        release_mem_region(((unsigned long)dpram_base_addr), dpram_size);
        return -1;
    }

    rtdm_printk("%s: DPRAM interface opened \n", DEVICE_NAME);

    // 이미 열렸음을 표시
    is_opened = 1;

    return 0;
}

// close() - 디바이스파일을 닫았을 경우에 호출
static void dpram_rtdm_close(struct rtdm_fd * fd)
{
    // 이미 닫혔을 경우에는 추가적인 설정을 안함
    if(is_opened != 1){
        rtdm_printk("%s: module already released\n", DEVICE_NAME);
        return;
    }

    // 할당된 가상주소 해제
    iounmap(dpram_virt_addr);

    // 물리적 주소에 대한 접근을 해제
    release_mem_region(((unsigned long)dpram_base_addr), dpram_size);

    rtdm_printk("%s: Release: module released\n", DEVICE_NAME);
    is_opened = 0;

}


// insmod dpram.ko 를 하였을 경우에 호출
static int __init dpram_init(void)
{
    int i;

    rtdm_printk(KERN_INFO "%s: device driver is initialized\n", DEVICE_NAME);

    // 디버깅용 LED의 핀번호가 valid한지 체크
    for(i = 0; i < dpram_gpio_pin_size; ++i){
        if (!gpio_is_valid(dpram_gpio_pin_numbers[i])){
            rtdm_printk(KERN_INFO "%s: gpio: invalid LED GPIO\n", DEVICE_NAME);
            return -ENODEV;
        }
    }

    // 디버깅용 LED의 GPIO 설정
    for(i = 0; i < dpram_gpio_pin_size; ++i){
        dpram_gpio_turn_on[i] = true;
        gpio_request(dpram_gpio_pin_numbers[i], "sysfs");          // 해당 GPIO 핀의 사용을 요청
        gpio_direction_output(dpram_gpio_pin_numbers[i], dpram_gpio_turn_on[i]);   // 해당 GPIO 핀의 입출력 설정
        gpio_export(dpram_gpio_pin_numbers[i], false);             // user 프로그램에서 바로 access할 수 있도록 export 함 (/sys/class/gpio/)
    }

    // DPRAM의 semaphore enable을 위한 gpio 설정
    gpio_request(dpram_gpio_sem_pin_number, "sysfs");          // 해당 GPIO 핀의 사용을 요청
    gpio_direction_output(dpram_gpio_sem_pin_number, true);    // 해당 GPIO 핀의 입출력 설정
    gpio_export(dpram_gpio_sem_pin_number, false);             // user 프로그램에서 바로 access할 수 있도록 export 함 (/sys/class/gpio/)

    // DPRAM의 chip enable을 위한 gpio 설정
    gpio_request(dpram_gpio_cen_pin_number, "sysfs");          // 해당 GPIO 핀의 사용을 요청
    gpio_direction_output(dpram_gpio_cen_pin_number, true);    // 해당 GPIO 핀의 입출력 설정
    gpio_export(dpram_gpio_cen_pin_number, false);             // user 프로그램에서 바로 access할 수 있도록 export 함 (/sys/class/gpio/)

    rtdm_printk(KERN_INFO "%s: device driver is successfully initialized\n", DEVICE_NAME);

    // register real-time device module
    //return rtdm_dev_register(&device);
    return rtdm_dev_register(&device[0]);
}

// rmmod dpram.ko 를 하였을 경우에 호출
static void __exit dpram_exit(void)
{
    int i;
    // unregister real-time device module
    //rtdm_dev_unregister(&device, 1000);
    rtdm_dev_unregister(&device[0]); // by skj

    for(i = 0; i < dpram_gpio_pin_size; ++i){
        gpio_set_value(dpram_gpio_pin_numbers[i], 0);
        gpio_unexport(dpram_gpio_pin_numbers[i]);         // user 프로그램에서 바로 access할 수 없도록 unexport 함 (/sys/class/gpio/)
        gpio_free(dpram_gpio_pin_numbers[i]);             // 해당 GPIO 핀의 사용을 해제
    }
    gpio_set_value(dpram_gpio_sem_pin_number, true);
    gpio_unexport(dpram_gpio_sem_pin_number);             // user 프로그램에서 바로 access할 수 없도록 unexport 함 (/sys/class/gpio/)
    gpio_free(dpram_gpio_sem_pin_number);                 // 해당 GPIO 핀의 사용을 해제

    gpio_set_value(dpram_gpio_cen_pin_number, true);
    gpio_unexport(dpram_gpio_cen_pin_number);             // user 프로그램에서 바로 access할 수 없도록 unexport 함 (/sys/class/gpio/)
    gpio_free(dpram_gpio_cen_pin_number);                 // 해당 GPIO 핀의 사용을 해제

    rtdm_printk(KERN_INFO "%s: device driver is removed\n", DEVICE_NAME);

} 


// DTS 파일의 설정을 읽어 올 수 있도록 compatible 설정
//   하지만, 본 디바이스 드라이버에서는 읽을 내용이 없으므로 큰필요가 없음.
static const struct of_device_id dpram_of_match[] = {
    { .compatible = DEVICE_NAME, },
    { },
};


MODULE_DEVICE_TABLE(of, dpram_of_match);   // compatible과 일치하는 로드된 DTS 파일 내용을 of에 매핑
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Deok-Hwa Kim <realsanai123@gmail.com>");

module_init(dpram_init);    // init 함수 설정
module_exit(dpram_exit);    // exit 함수 설정
