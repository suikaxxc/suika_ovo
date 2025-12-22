#ifndef _LIVING_ROMM_TASK_H_
#define _LIVING_ROMM_TASK_H_

extern int fire_alarm;
extern unsigned short fire_state; // 1，表示发生火灾，0表示未发生火灾
extern float humidity;            //用于保存湿度的变量
extern float temperature;         //用于保存温度的变量
void living_room_task(void);

#endif