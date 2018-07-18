# dockerexample 使用说明

- dockerexample是php molten扩展的docker示例。
- 通过dockerexample可以很快实践php molten

### 第一步、修改配置

在安装molten扩展时, 需要制定zipkin的服务器地址, 并指定API版本, 当然molten的其他配置也可以在docker-php-ext-molten.ini配置

```
#vim docker-php-ext-molten.ini
molten.sink_http_uri=http://zipkin_server_id:9411/api/v2/spans
```

### 第二步、启动环境
```
docker-compose up -d --build --force-recreate

# 通过docker-compose ps 可以看到容器状态
```

### 第三部、访问
> 修改public/*.php中的 demo_server_ip
> 修改ini中的 zipkin_server_id


```
ab -n 2560 -c 20 http://demo_server_ip:10400/
```

### 查看调用链

访问 http://zipkin_server_id:9411 就可以了.
