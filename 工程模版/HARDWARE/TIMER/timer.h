#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
 	
/****调度器开始**********************************************/
#define MAXTASKS 3
extern volatile unsigned char timers[MAXTASKS];
#define _SS static unsigned char _lc; switch(_lc){default: 
#define _EE ;}; _lc=0; return 255;
#define WaitX(tickets)  do {_lc=__LINE__; return tickets ;} while(0); case __LINE__: 

#define RunTask(TaskName,TaskID) do { if (timers[TaskID]==0) timers[TaskID]=TaskName(); }  while(0); 
#define RunTaskA(TaskName,TaskID) { if (timers[TaskID]==0) {timers[TaskID]=TaskName(); continue;} }   //前面的任务优先保证执行

#define CallSub(SubTaskName) do {unsigned char currdt; _lc=__LINE__; return 0; case __LINE__:  currdt=SubTaskName(); if(currdt!=255) return currdt;} while(0);
#define UpdateTimers() {unsigned char i; for(i=MAXTASKS;i>0 ;i--){if((timers[i-1]!=0)&&(timers[i-1]!=255)) timers[i-1]--;}}

#define SEM unsigned int 
//初始化信号量
#define InitSem(sem) sem=0;
//等待信号量
#define WaitSem(sem) do{ sem=1; WaitX(0); if (sem>0) return 1;} while(0);
//等待信号量或定时器溢出， 定时器tickets 最大为0xFFFE
#define WaitSemX(sem,tickets)  do { sem=tickets+1; WaitX(0); if(sem>1){ sem--;  return 1;} } while(0);
//发送信号量
#define SendSem(sem)  do {sem=0;} while(0);

void TIM3_Int_Init(u16 arr,u16 psc);
#endif
