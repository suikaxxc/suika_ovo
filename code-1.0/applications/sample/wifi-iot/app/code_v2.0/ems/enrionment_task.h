#ifndef ENRIONMENT_TASK_H
#define ENRIONMENT_TASK_H

extern float humidity;      // 湿度值
extern float temperature;   // 温度值
extern unsigned short gas;  // 可燃气体值
extern unsigned short stat; // 0正常，1，警告;2，危险

void enrionment_task(void); // 子模块线程函数
#endif