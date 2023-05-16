#ifndef __DPRAM_DEV_H__
#define __DPRAM_DEV_H__

#include <linux/ioctl.h>

#define DPRAM_ID_NUM (100)
#define CONTROL_JOINT_NUM (10)
#define DPRAM_MAX_SIZE (0x1000)

typedef struct
{
	double torque[CONTROL_JOINT_NUM];
	double velocity[CONTROL_JOINT_NUM];
	double position[CONTROL_JOINT_NUM];
} control_data;

typedef struct
{
	double torque[CONTROL_JOINT_NUM];
	double velocity[CONTROL_JOINT_NUM];
	double position[CONTROL_JOINT_NUM];
} state_data;

#define DPRAM_SEND_CONTROL_DATA			_IOW( DPRAM_ID_NUM, 0, control_data )
#define DPRAM_RECV_STATE_DATA			_IOR( DPRAM_ID_NUM, 1, state_data )
		
#define DPRAM_RECV_TEST_CONTROL_DATA		_IOR( DPRAM_ID_NUM, 2, control_data )
#define DPRAM_SEND_TEST_STATE_DATA		_IOW( DPRAM_ID_NUM, 3, state_data )

#endif
