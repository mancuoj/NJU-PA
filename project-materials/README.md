#  笔记

## pa0

安装工具和源代码

```sh
sudo apt install build-essential man gcc-doc gdb git libreadline-dev libsdl2-dev llvm llvm-dev llvm-11 llvm-11-de
git clone -b 2022 git@github.com:NJU-ProjectN/ics-pa.git ics2022
git branch -m master # 修改分支名，我用的 main
bash init.sh nemu
bash init.sh abstract-machine
source ~/.bashrc # 其他 shell 把两行变量写入配置文件即可，我这里写到 .zshrc 中
```

运行

```sh
cd nemu
make menuconfig # 发现缺少 flex，安装即可
make
```

其他命令

```sh
make run # 暂不可用
make clean
make gdb
```

运行后会自动追踪并提交代码，`git log` 查看，如果不想可以注释掉 Makefile 里的 `define git_commit` 相关内容

## pa1

http://jyywiki.cn/ICS/2021/labs/PA1 下载 rom 试着运行

将游戏 ROM 放置在`nes/rom/`目录下，然后可通过 `mainargs` 选择运行的游戏, 如:

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

测试按键

```sh
bash init.sh am-kernels
cd am-kernels/tests/am-tests
make ARCH=native mainargs=k run
```

加快编译速度

```sh
lscpu # 查看 CPU 数量
make -j8 # 8 个就是 j8
time make # 查看 total 编译时间
```

使用 ccache

```sh
sudo apt install ccache
man ccache # 查看 usage

export PATH="/usr/lib/ccache:$PATH" # 添加到 shell 配置里
which gcc # 应该输出 /usr/lib/ccache/gcc
```

