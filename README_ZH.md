
# molten

[![Build Status](https://travis-ci.org/chuan-yun/Molten.svg?branch=master)](https://travis-ci.org/chuan-yun/Molten)

molten是应用透明链路追踪工具。

molten追踪php核心调用库运行时信息并且按照zipkin/optracing格式输出信息。 

molten提供多种sapi, 多种采样类型, 上报追踪状态, 模块控制和多种数据落地 
类型等功能。

依赖于molten很容易构建基于php语言的[分布式全链路追踪系统](https://research.google.com/pubs/pub36356.html) 
目前已经运行在生产环境上千台机器上。

# 目录
----
1. [安装](#安装)
2. [快速开始](#快速开始)
3. [配置](#配置)
4. [验证](#验证)
5. [功能](#功能)
6. [贡献](#贡献)
6. [许可](#许可)


# 安装

以下是你需要做的安装molten在你的系统上。

~~~
phpize
./configure
make && make install
~~~

`make install` 复制 `molten.so` 到确切的位置, 但是你还需要开启模块在php配置中,编辑你自己的php.ini或者添加molten.ini在`/etc/php5/conf.d`, 并且添加如下内容:`extension=molten.so`。

# 快速开始

~~~
cd example
sh run.sh
~~~
在浏览器中打开 `http://127.0.0.1:9411/zipkin/`, 能够看见链路信息。

`注意` 如果没有看到详细信息，那么EndTime选项中添加1小时。

# 配置
## 基础配置

`molten.enable` 1开启0关闭, 默认 `1`。

`molten.service_name` 设置应用服务名, 默认`default`。

`molten.tracing_cli` 1追踪cli模式下信息, 0关闭, 默认`0`。

## 采样配置

`molten.sampling_type` 类型类型， 1采样率控制, 2通过每分钟request数, 默认是`1`。

`molten.sampling_request` 采样类型是请求数采样，每分钟的采样请求数, 默认是`10`。

`molten.sampling_rate_base` 采样类型是采样率时，每个请求的采样几率, 默认是`256`。

## 控制模块配置

`molten.ctrl_domain_path` 控制模块unix通信地址 `/tmp/tracing.sock`。

`molten.ctrl_call_interval` 控制模块调用间隔, 默认 `60`。

## 上报模块配置

上报模块使用和数据模块相同的输出类型

`molten.report_interval` 数据模块调用间隔, 默认 `60`。

`molten.report_limit` 数据上报请求上限, 默认 `100`。


## 数据模块

`molten.sink_type` 数据落地类型, `1` 写入文件, 文件地址依赖`molten.sink_log_path`, `2` 写入到标准输出, `4` 通过curl发送, 发送地址依赖 `molten.sink_http_uri`.

`molten.output_type`  输出全部追踪块(span)(`1`) 或者一行输出一个块(`2`)。

`molten.sink_log_path` 写入文件地址。

`ptrcing.sink_http_uri` 发送http地址。

## 追踪块配置

`molten.span_format` 追踪块格式(span), 不同的追踪系统选择`zipkin` 或者 `opentracing`。


# 验证

```shell
php -d molten.sink_type=2 -d molten.enable_cli=1 -r '$c=curl_init("http://localhost:12345");curl_exec($c);'
```
可以看到如下输出：
```
[{"traceId":"%s","name":"php_curl","version":"php-4","id":"1.1","parentId":"1","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"%s","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"%s","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:12345\/","endpoint":{"serviceName":"%s","ipv4":"%s"}},{"key":"error","value":"Failed
connect to localhost:12345; Connection
refused","endpoint":{"serviceName":"%s","ipv4":"%s"}}]},{"traceId":"%s","name":"cli","version":"php-4","id":"1","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"%s","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"%s","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"-","endpoint":{"serviceName":"%s","ipv4":"%s"}}]}]
```

# 功能

从上述配置中，你可以看到我们提供的功能。

## 拦截器

molten 拦截 curl,pdo,mysqli,redis,mongodb,memcached扩展，构建运行时追踪信息. 支持全链路追踪功能, molten 替换了curl_exec,curl_setopt,curl_setopt_array函数， 并且在请求中添加了链路头(x-w-traceid, x-w-spanid and so on)。

定制化的链路格式， 支持两个流行格式(`zipkin` 和 `opentracing`)。

## 采样

两种不同的采样方式并且能够通过控制模块进行修改。

## 数据落地

molten当前支持3种数据落地方式，标准输出，文件，http。并且能够选择输出的位置。

## 控制

使用agent工具能够控制molten的工作方式。

## 上报

上报模块能够记录，molten并没有采样样到的关键数据信息。

# Contributing

欢迎开发者们使molten变得更好。

您可以联系如下邮件列表来讨论。

phobosw@gmail.com

silkcutbeta@gmail.com

你还可以通过以下方式贡献。

* [提交issue](https://github.com/chuan-yun/Molten/issues)。
* 通过pull request提交修改和功能。

# 协议

Apache License Version 2.0 see http://www.apache.org/licenses/LICENSE-2.0.html
