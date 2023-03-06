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

#include <isa.h>
#include <memory/paddr.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() {
  /* CONFIG_xxx 都定义在在 autoconf.h 中，CONFIG_TRACE 被定义为 1 */
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  /*
  - __TIME__ 宏表示程序的编译时间，格式为 "hh:mm:ss"
  - __DATE__ 宏表示程序的编译日期，格式为 "mmm dd yyyy"
  */
  Log("Build time: %s, %s", __TIME__, __DATE__);
  /*
  - __GUEST_ISA__ 被定义在 $(CFLAGS) 中、
  - 黄色前景色和红色背景色
  */
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;

static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  /* rb 就是 read binary 以二进制字节为单位进行读取 */
  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  /* 从文件末尾 SEEK_END 偏移 0 个字节，也就是将文件指针移至文件末尾 */
  fseek(fp, 0, SEEK_END);
  /* ftell 计算文件指针距离文件开头处的偏移量 */
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  /* 移回开头处 */
  fseek(fp, 0, SEEK_SET);
  /* 读取 1 个 size 大小的块存到 PC 中，也就是将映像内容存到 PC 中，并返回实际读取的块的数量 */
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp);
  return size;
}

static int parse_args(int argc, char *argv[]) {
  /* 
  - 表示程序所支持的命令行选项信息，长短名称 
  - 也是 getopt_long() 所需的第四个参数结构体
  - 具体可 ctrl 点击 option 查看定义
  */
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  /* 
  - getopt_long(1, 2, 3, 4, 5)
    1，2 为传入的命令行参数
    3 是短选项列表，每个字符都代表一个短选项
      -bh 表示不需要参数
      :l, :d, :p 表示需要参数
    4 是长选项列表
    5 是一个指向 int 类型的变量的指针，它用于存储 getopt_long 函数返回的长选项的索引
  - 读取所有命令行选项后会返回 -1，所以一般使用以下的 while 循环来读取
  - getopt_long() 会将命令行选项的参数存储在 C 语言的全局变量 optarg 中
  */
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      /* 使用非选项参数就会触发 case 1，将其设置为 imge_file 的值 */
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. 进行一些全局初始化 */

  /* Parse arguments. 解析运行程序时传入的命令行参数 */
  parse_args(argc, argv);

  /* Set random seed. 设置随机数种子 */
  init_rand();

  /* Open the log file. 初始化日志文件，通过命令行参数传递 */
  init_log(log_file);

  /* Initialize memory. 初始化内存，在 paddr.c 中 */
  init_mem();

  /* Initialize devices. 初始化设备，在 device.c 中 */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. 指令集相关的初始化，默认是 riscv32，在 init.c 中 */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. 加载映像到内存中，覆盖内置映像 */
  long img_size = load_img();

  /* Initialize differential testing. 初始化差分测试 */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. 初始化调试器，在 sdb.c 中 */
  init_sdb();

  /* CONFIG_ITRACE 为 1，ISA 默认为 riscv32，初始化对应的反汇编器 */
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv32, "riscv32",
    MUXDEF(CONFIG_ISA_riscv64, "riscv64", "bad")))) "-pc-linux-gnu"
  ));

  /* Display welcome message. 展示欢迎信息 */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
