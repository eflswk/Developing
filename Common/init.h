/*
 * 文件名：init.h
 * 作  用：
 *      系统初始化接口文件。
 *
 *      1. 提供统一的初始化函数声明
 *      2. 对外暴露系统初始化入口
 *      3. 便于模块化管理初始化流程
 */
 
#ifndef __INIT_H
#define __INIT_H

#include "common.h"

/* ===================== 初始化接口 ===================== */

/* 系统初始化 */
void System_Init(void);

#endif	// !__INIT_H
