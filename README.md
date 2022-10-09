# MyDSS

兼容 `Redis` 协议的服务器。

## 构建

```bash
git clone https://github.com/VincilLau/mydss.git
cd mydss
xmake f -m release
xmake build
```

## 支持的命令

### 通用

- DEL
- EXISTS
- EXPIRE
- EXPIREAT
- OBJECT
- PERSIST
- PEXPIRE
- PEXPIREAT
- PTTL
- RENAME
- RENAMENX
- TOUCH
- TTL
- TYPE

### 连接管理

- CLIENT GETNAME
- CLIENT ID
- CLIENT SETNAME
- ECHO
- PING
- QUIT
- SELECT

### 字符串

- APPEND
- DECR
- DECRBY
- GET
- GETDEL
- GETRANGE
- INCR
- INCRBY
- MGET
- MSET
- MSETNX
- SET
- STRLEN

## 维护者

[@Vincil Lau](https://github.com/VincilLau).

## 许可证

[Apache 2.0](LICENSE).

