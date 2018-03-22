# demo

### 修改配置
```
#vim docker-php-ext-molten.ini
molten.sink_http_uri=http://zipkin_server_id:9411/api/v2/spans
```

### 启动环境
```
docker-compose up -d --build --force-recreate
```

### 访问
> 修改public/*.php中的 demo_server_ip
> 修改ini中的 zipkin_server_id


```
ab -n 2560 -c 20 http://demo_server_ip:10400/
```