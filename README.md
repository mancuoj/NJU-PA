# ICS2022 Programming Assignment

This project is the programming assignment of the class ICS(Introduction to Computer System)
in Department of Computer Science and Technology, Nanjing University.

For the guide of this programming assignment,
refer to https://nju-projectn.github.io/ics-pa-gitbook/ics2022/

To initialize, run
```bash
bash init.sh subproject-name
```
See `init.sh` for more details.

The following subprojects/components are included. Some of them are not fully implemented.

* [NEMU](https://github.com/NJU-ProjectN/nemu)
* [Abstract-Machine](https://github.com/NJU-ProjectN/abstract-machine)
* [Nanos-lite](https://github.com/NJU-ProjectN/nanos-lite)
* [Navy-apps](https://github.com/NJU-ProjectN/navy-apps)


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