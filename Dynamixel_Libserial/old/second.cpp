#include <stdio.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
//#include <dynamixel-rtu.h>
#include <dynamixel.h>
#include "dynamixel.h"
#include <dynamixel-rtu.h>
//#include "config.h"


#define DYN_CFG_PORT	"/dev/ttyACM0"
#define DYN_CFG_SPEED	115200
#define DYN_CFG_DEBUG	true

#define ID	1
#define DYN_CFG_REGISTER	DYNAMIXEL_R_LED



int main(void) {
	uint8_t *data;
	uint8_t state;
	uint8_t req_data[1]={0};

	dynamixel_t *dyn;
	dyn = dynamixel_new_rtu(DYN_CFG_PORT, DYN_CFG_SPEED, _DYNAMIXEL_SERIAL_DEFAULTS);
	dynamixel_set_debug(dyn,true);

	state=dynamixel_write_data(dyn,ID,DYN_CFG_REGISTER,1,req_data);
	state=dynamixel_read_data(dyn,ID,DYN_CFG_REGISTER,1,&data);
	if (dynamixel_connect(dyn)==0) {
		if (state>0) {
		//	printf("Register value: ");
		//	printf("<%02x>",*(data++));
	//	printf("\n");
		}
		if (req_data[0]==0) {
			req_data[0]=1;
		} else {
			req_data[0]=0;
		}

		uint8_t zero[1] = {0x00};
		uint8_t t[1] = {0xF};
		int i = 0;
		while (i < 10) {		
			printf("Looping\n");
			dynamixel_reg_write_byte(dyn, ID, DYNAMIXEL_R_GOAL_POSITION_L, 0x0);
			dynamixel_reg_write_byte(dyn, ID, DYNAMIXEL_R_GOAL_POSITION_H, 0x0);
			usleep(100000);
			dynamixel_reg_write_byte(dyn, ID, DYNAMIXEL_R_GOAL_POSITION_L, 0xF);
			usleep(100000);
			dynamixel_reg_write_byte(dyn, ID, DYNAMIXEL_R_GOAL_POSITION_H, 0xF);

			dynamixel_reg_write_byte(dyn, ID, DYNAMIXEL_R_LED, 1);
			i++;
		}

		dynamixel_close(dyn);
	}
	dynamixel_free(dyn);
}
