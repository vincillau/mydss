# MyDSS

![Version](https://img.shields.io/github/v/release/vincillau/mydss)
[![License](https://img.shields.io/github/license/vincillau/mydss)](LICENSE)
[![Build](https://github.com/VincilLau/mydss/actions/workflows/build.yml/badge.svg?branch=dev)](https://github.com/VincilLau/mydss/actions/workflows/build.yml)
[![test](https://github.com/VincilLau/mydss/actions/workflows/test.yml/badge.svg?branch=dev)](https://github.com/VincilLau/mydss/actions/workflows/test.yml)

兼容 `Redis` 协议的服务器，目前仅支持 Linux。

## 构建

MyDSS 使用 [xmake](https://xmake.io) 构建项目和管理依赖

```bash
git clone https://github.com/VincilLau/mydss.git
cd mydss
git checkout master
xmake f -m release
xmake build
```

## 运行

```bash
xmake run mydss
iredis # 也可以使用 redis-cli 等工具
# SET k v
# GET k
```

查看帮助：

```bash
xmake run mydss --help
```

指示配置文件路径：

```bash
xmake run mydss -c /path/to/config.json
```

注意：使用 xmake run 运行时工作目录不再项目根目录中，使用相对路径指示配置文件可能出错，可以改成如下方式：

```bash
./build/linux/x86_64/release/mydss -c ./config.json
```

## 文档

- [命令列表](docs/commands.md)
- [配置文件格式](docs/config.md)

## 维护者

[@Vincil Lau](https://github.com/VincilLau)

## 许可证

[Apache 2.0](LICENSE)

