# pa1

## 试运行游戏

下载 rom：http://jyywiki.cn/ICS/2021/labs/PA1

将游戏 ROM 放置在 `nes/rom/` 目录下，然后可通过 `mainargs` 选择运行的游戏, 如:

```sh
cd fceux-am
make ARCH=native run mainargs=mario
```

操作方式

* U — SELECT
* I — START
* J — A键
* K — B键
* W/S/A/D — UP/DOWN/LEFT/RIGHT
* Q — 退出


```sh
# 测试按键
bash init.sh am-kernels
cd am-kernels/tests/am-tests
make ARCH=native mainargs=k run
```

## 加快编译速度

```sh
lscpu # 查看 CPU 数量
make -j8 # 8 个就是 j8
time make # 查看 total 编译时间
```


```sh
# 使用 ccache
sudo apt install ccache
man ccache # 查看 usage

export PATH="/usr/lib/ccache:$PATH" # 添加到 shell 配置里
which gcc # 应该输出 /usr/lib/ccache/gcc
```

## 程序是个状态机

计算 1 + 2 + ... + 100

```
// PC: instruction    | // label: statement
0: mov  r1, 0         |  pc0: r1 = 0;
1: mov  r2, 0         |  pc1: r2 = 0;
2: addi r2, r2, 1     |  pc2: r2 = r2 + 1;
3: add  r1, r1, r2    |  pc3: r1 = r1 + r2;
4: blt  r2, 100, 2    |  pc4: if (r2 < 100) goto pc2;   // branch if less than
5: jmp 5              |  pc5: goto pc5;
```

(PC, r1, r2)，x 代表未初始化，PC 执行完后会指向下一条指令，写出前两次循环和最后两次循环的状态机

```
(0, x, x) 
-> pc0(1, 0, x) -> pc1(2, 0, 0) -> pc2(3, 0, 1) -> pc3(4, 1, 1) -> pc4(2, 1, 1) 
-> pc2(3, 1, 2) -> pc3(4, 3, 2) 
-> ...
-> pc2(3, 4851, 99) -> pc3(4, 4950, 99) -> pc4(2, 4950, 99)
-> pc2(3, 4950, 100) -> pc3(4, 5050, 100) -> pc4(5, 5050, 100) -> pc5(5, 5050, 100) -> ....
```

## 项目框架

```
ics2022
├── abstract-machine   # 抽象计算机
├── am-kernels         # 基于抽象计算机开发的应用程序
├── fceux-am           # 红白机模拟器
├── init.sh            # 初始化脚本
├── Makefile           # 用于工程打包提交
├── nemu               # NEMU
└── README.md
```

NEMU 主要由 `monitor, CPU, memory, 设备` 四个模块构成，monitor 的引入是为了方便调试，监控客户计算机的运行状态，其代码框架如下：

```
nemu
├── configs                    # 预先提供的一些配置文件
├── include                    # 存放全局使用的头文件
│   ├── common.h               # 公用的头文件
│   ├── config                 # 配置系统生成的头文件, 用于维护配置选项更新的时间戳
│   ├── cpu
│   │   ├── cpu.h
│   │   ├── decode.h           # 译码相关
│   │   ├── difftest.h
│   │   └── ifetch.h           # 取指相关
│   ├── debug.h                # 一些方便调试用的宏
│   ├── device                 # 设备相关
│   ├── difftest-def.h
│   ├── generated
│   │   └── autoconf.h         # 配置系统生成的头文件, 用于根据配置信息定义相关的宏
│   ├── isa.h                  # ISA相关
│   ├── macro.h                # 一些方便的宏定义
│   ├── memory                 # 访问内存相关
│   └── utils.h
├── Kconfig                    # 配置信息管理的规则
├── Makefile                   # Makefile构建脚本
├── README.md
├── resource                   # 一些辅助资源
├── scripts                    # Makefile构建脚本
│   ├── build.mk
│   ├── config.mk
│   ├── git.mk                 # git版本控制相关
│   └── native.mk
├── src                        # 源文件
│   ├── cpu
│   │   └── cpu-exec.c         # 指令执行的主循环
│   ├── device                 # 设备相关
│   ├── engine
│   │   └── interpreter        # 解释器的实现
│   ├── filelist.mk
│   ├── isa                    # ISA相关的实现
│   │   ├── mips32
│   │   ├── riscv32
│   │   ├── riscv64
│   │   └── x86
│   ├── memory                 # 内存访问的实现
│   ├── monitor
│   │   ├── monitor.c
│   │   └── sdb                # 简易调试器
│   │       ├── expr.c         # 表达式求值的实现
│   │       ├── sdb.c          # 简易调试器的命令处理
│   │       └── watchpoint.c   # 监视点的实现
│   ├── nemu-main.c            # 你知道的...
│   └── utils                  # 一些公共的功能
│       ├── log.c              # 日志文件相关
│       ├── rand.c
│       ├── state.c
│       └── timer.c
└── tools                      # 一些工具
    ├── fixdep                 # 依赖修复, 配合配置系统进行使用
    ├── gen-expr
    ├── kconfig                # 配置系统
    ├── kvm-diff
    ├── qemu-diff
    └── spike-diff
```

ISA 相关都在 `nemu/src/isa` 目录下，通过 `nemu/include/isa.h` 提供相关 API

## Kconfig

配置系统 kconfig 定义了一套简单的语言来编写配置描述文件，在 NEMU 项目中, "配置描述文件"的文件名都为 Kconfig, 如 `nemu/Kconfig`

当你键入 `make menuconfig` 的时候, 背后其实发生了如下事件:

- 检查 `nemu/tools/kconfig/build/mconf` 程序是否存在, 若不存在, 则编译并生成 mconf
- 检查 `nemu/tools/kconfig/build/conf` 程序是否存在, 若不存在, 则编译并生成 conf
- 运行命令 `mconf nemu/Kconfig`, 此时 mconf 将会解析 `nemu/Kconfig` 中的描述, 以菜单树的形式展示各种配置选项, 供开发者进行选择
- 退出菜单时, `mconf` 会把开发者选择的结果记录到 `nemu/.config` 文件中
- 运行命令 `conf --syncconfig nemu/Kconfig` , 此时 conf 将会解析 `nemu/Kconfig` 中的描述, 并读取选择结果 `nemu/.config`, 结合两者来生成如下文件:
  - 可以被包含到 C 代码中的宏定义 `nemu/include/generated/autoconf.h`, 这些宏的名称都是形如 `CONFIG_xxx` 的形式
  - 可以被包含到 Makefile 中的变量定义 `nemu/include/config/auto.conf`
  - 可以被包含到 Makefile 中的, 和"配置描述文件"相关的依赖规则 `nemu/include/config/auto.conf.cmd`, 为了阅读代码, 我们可以不必关心它
  - 通过时间戳来维护配置选项变化的目录树 `nemu/include/config/`, 它会配合另一个工具 `nemu/tools/fixdep` 来使用, 用于在更新配置选项后节省不必要的文件编译, 为了阅读代码, 我们可以不必关心它

现在只需要关心 `nemu/include/generated/autoconf.h` 和 `nemu/include/config/auto.conf` 这两个文件即可

## Makefile

Makefile 通过包含 `nemu/include/config/auto.conf` , 与 kconfig 生成的变量进行关联. 因此在通过 menuconfig 更新配置选项后, Makefile 的行为可能也会有所变化

`nemu/src/filelist.mk` 用于指定最终会参与编译的源文件，会根据 menuconfig 的配置来维护如下 4 个变量：

- `SRCS-y` - 参与编译的源文件的候选集合
- `SRCS-BLACKLIST-y` - 不参与编译的源文件的黑名单集合
- `DIRS-y` - 参与编译的目录集合, 该目录下的所有文件都会被加入到 SRCS-y 中
- `DIRS-BLACKLIST-y` - 不参与编译的目录集合, 该目录下的所有文件都会被加入到 SRCS-BLACKLIST-y 中

上述4个变量还可以与 `menuconfig` 的配置结果中的布尔选项进行关联, 例如 `DIRS-BLACKLIST-$(CONFIG_TARGET_AM) += src/monitor/sdb`, 这样当我们在 menuconfig 中选择了 `TARGET_AM` 选项时, 会得到 `DIRS-BLACKLIST-y += src/monitor/sdb`

所以当我们选中 `TARGET_AM` 选项时, `nemu/src/monitor/sdb` 目录下的所有文件都不会被编译


## 编译和链接

通过 `make -nB` 查看 make 过程中的命令, 可以看到如下命令:

```sh
echo + CC src/nemu-main.c
mkdir -p /(...)/nemu/build/obj-riscv32-nemu-interpreter/src/
gcc -O2 -MMD -Wall -Werror -I/(...)/nemu/include -I/(...)/nemu/src/engine/interpreter -I/(...)/nemu/src/isa/riscv32/include -O2    -DITRACE_COND=true -D__GUEST_ISA__=riscv32 -c -o /(...)/nemu/build/obj-riscv32-nemu-interpreter/src/nemu-main.o src/nemu-main.c
...
```

编译规则在 `nemu/scripts/build.mk` 中定义：

```
$(OBJ_DIR)/%.o: %.c
  @echo + CC $<
  @mkdir -p $(dir $@)
  @$(CC) $(CFLAGS) -c -o $@ $<
  $(call call_fixdep, $(@:.o=.d), $@)
```

## 第一个客户程序

需要调用 `init_monitor()` 函数来进行初始化工作，将客户程序读入到客户计算机中

`make run` 查看到错误信息，打印了

```
[src/monitor/monitor.c:44 welcome] Exercise: Please remove me in the source code and compile NEMU again.
```

找到以下代码删除即可

```c
-- Log("Exercise: Please remove me in the source code and compile NEMU again.");
-- assert(0);
```

初始化完成后，main() 继续调用 engine_start() 进入调试器的主循环 sdb_mainloop()

> 我们已经在上文提到过, kconfig 会根据配置选项的结果在 `nemu/include/generated/autoconf.h` 中定义一些形如 `CONFIG_xxx` 的宏, 我们可以在 C 代码中通过条件编译的功能对这些宏进行测试, 来判断是否编译某些代码. 例如, 当 `CONFIG_DEVICE` 这个宏没有定义时, 设备相关的代码就无需进行编译
> 为了编写更紧凑的代码, 我们在 `nemu/include/macro.h` 中定义了一些专门用来对宏进行测试的宏. 例如 `IFDEF(CONFIG_DEVICE, init_device());` 表示, 如果定义了 `CONFIG_DEVICE`, 才会调用 `init_device()` 函数; 而 `MUXDEF(CONFIG_TRACE, "ON", "OFF")` 则表示, 如果定义了 `CONFIG_TRACE`, 则预处理结果为"ON"("OFF"在预处理后会消失), 否则预处理结果为"OFF"
> 这些宏的功能非常神奇, 你知道这些宏是如何工作的吗?

？

> 阅读 `init_monitor()` 函数的代码, 你会发现里面全部都是函数调用. 按道理, 把相应的函数体在 `init_monitor()` 中展开也不影响代码的正确性. 相比之下, 在这里使用函数有什么好处呢?

更易维护、修改和测试

> `parse_args()` 中调用了一个你也许不太熟悉的函数 `getopt_long()`, 框架代码通过它来对参数进行解析, 具体的行为可以查阅 `man 3 getopt_long`

```c
#include <getopt.h>

// 解析命令行参数
int getopt_long(int argc, char * const argv[],
       const char *optstring,
       const struct option *longopts, int *longindex);
```

- man 1 用于查看用户命令
- man 2 用于查看系统调用
- man 3 用于查看函数库
- man 4 用于查看特殊文件和设备
- ...

> 另外的一个问题是, 这些参数是从哪里来的呢?

native.mk 中定义的

> 在 `cmd_c()` 函数中, 调用 `cpu_exec()` 的时候传入了参数 -1, 你知道这是什么意思吗?

执行单条指令直到遇到下个断点或者执行了指定次数

> "调用 `cpu_exec()` 的时候传入了参数 -1", 这一做法属于未定义行为吗? 请查阅 C99 手册确认你的想法

