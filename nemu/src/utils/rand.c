/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#ifndef CONFIG_TARGET_AM
#include <time.h>
#endif

/*
- 如果定义了 CONFIG_TARGET_AM，就用 0 作为随机数种子，否则用 time(0) 作为随机数种子
- time(0) 会返回当前时间
*/
void init_rand() {
  srand(MUXDEF(CONFIG_TARGET_AM, 0, time(0)));
}
