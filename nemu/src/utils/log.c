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

extern uint64_t g_nr_guest_inst;
FILE *log_fp = NULL;

void init_log(const char *log_file) {
  /* 如果 log_file 不为空，就打开文件，否则就用 stdout */
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    /* 用断言在文件打开错误立即停止运行？*/
    Assert(fp, "Can not open '%s'", log_file);
    log_fp = fp;
  }
  /* Log 函数定义在 debug.h 中*/
  Log("Log is written to %s", log_file ? log_file : "stdout");
}

/*
- MUXDEF 如果第一个参数被定义了，就返回第二个参数，否则返回第三个参数也就是不启用日志记录
- CONFIG_TRACE 在 autoconf.h 中根据配置文件定义为 1
- 第二个参数分别检查当前值是否位于定义的起始值和结束值之间，如果是，则启用
*/
bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
         (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}
