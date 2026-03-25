#ifndef __MONITOR_TASK_H
#define __MONITOR_TASK_H

#include "common.h"

#if ( configENABLE_DEBUG_STACK_MONITOR == 1 )
/* 监察周期，单位：ms */
/*
 * 函数名：MonitorTask_Create
 * 作  用：创建监察任务
 */
void MonitorTask_Create(void);

/*
 * 函数名：MonitorTask_Entry
 * 作  用：监察任务入口函数
 */
void MonitorTask_Entry(void *argument);
#endif // !configENABLE_DEBUG_STACK_MONITOR

#endif	// !__MONITOR_TASK_H
