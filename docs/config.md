# 配置文件格式

```json
{
  // 服务器配置
  "server": [
    {
      "type": "ipv4", // 地址类型
      "ip": "127.0.0.1", // IP 地址
      "port": 6379, // 端口
      "backlog": 512 // listen 的 backlog 参数
    }
  ],
  // 数据库配置
  "db": {
    "db_num": 16 // 数据库数目
  }
}
```
