# molten

molten is transparency tool for application tracing it self module call.

It trace php app core call and output zipkin/opentracing format trace log. 

Provides features about muliti trace sapi, multi sampling type, upload tracing 
status, module control and muliti sink type.

It very easy to build a [distributed systems tracing infrastructure](https://research.google.com/pubs/pub36356.html) 
base on php, already run on thousand instance on production env.

# Table of contents
----
1. [Installing](#install)
2. [QuickStart](#quickstart)
3. [Configure](#configure)
4. [Verify](#verify)
5. [Features](#features)
6. [Contributing](#contributing)
6. [License](#license)


# Installing

Everything you should need to install molten on your system.

~~~
phpize
./configure
make && make install
~~~

`make install` copies `molten.so` to and appropriate location, but you still need to enable the module int the PHP config file, To do so, either edit your php.ini or add ad redis.ini file in `/etc/php5/conf.d` with the following contents:`extension=molten.so`.

# QuickStart

~~~
cd example
sh run.sh
~~~
open `http://127.0.0.1:9411/zipkin/` in your browser, you can see the tracing detail in it.

`NOTICE` if you not see the trace, you select EndTime +1 hour.

# Configure
## Base Config

`molten.enable` set 1 enable or set 0 disable, default `1`.

`molten.service_name` is to set service name for label app service name.

`molten.tracing_cli` set 1 to trace, cli sapi, 0 not trace, just use for trace unit test, default `0`.

## Sampling Config

`molten.sampling_type` sampling type choose to use sampling by rate(`1`) or request(`2`), default is `1`;

`molten.sampling_request` sampling by request, set to per min request num, defualt is `10`.

`molten.sampling_rate_base` determine a request sampled or not by rate, default is `256`.

## Control Config

`molten.ctrl_domain_path` control module unix sock path, default `/tmp/tracing.sock`.

`molten.ctrl_call_interval` control module pull config interval second, default `60`.

## Report Config

report module output type is same as sink module

`molten.report_interval` reporter call interval, default `60`.

`molten.report_limit`  reporter list limit current only for error num, default `100`.


## Sink Config

`molten.sink_type` sink type, `1` write log, log path is depend on `molten.sink_log_path`, `2` write log to standand output, `4` use curl to send trace log, http uri depend on `molten.sink_http_uri`.

`molten.output_type`  output spans on one line(`1`) or one line one span(`2`).

`molten.sink_log_path` locate log path.

`ptrcing.sink_http_uri` locate log http uri.

## Spans Config

`molten.span_format` span format, you can select `zipkin` or `opentracing` or different tracing system.


# Verify

```shell
php -d molten.sink_type=2 -d molten.enable_cli=1 -r '$c=curl_init("http://localhost:12345");curl_exec($c);'
```
You can see output below
```
[{"traceId":"%s","name":"php_curl","version":"php-4","id":"1.1","parentId":"1","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"%s","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"%s","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:12345\/","endpoint":{"serviceName":"%s","ipv4":"%s"}},{"key":"error","value":"Failed
connect to localhost:12345; Connection
refused","endpoint":{"serviceName":"%s","ipv4":"%s"}}]},{"traceId":"%s","name":"cli","version":"php-4","id":"1","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"%s","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"%s","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"-","endpoint":{"serviceName":"%s","ipv4":"%s"}}]}]
```

# Features

the Config Block above, you can see which feature we support.

## Interceptor

we intercept curl,pdo,mysqli,redis,mongodb,memcached extension to build execute time span info. for chain http request, we replace curl_exec,curl_setopt,curl_setopt_array to add http request trace header (x-w-traceid, x-w-spanid and so on).

the span_format is the way to custom span format, for two popular kinds (`zipkin` and `opentracing`)

## Sampling

different sampling type and change parameter to control sampling,  rate or request.

## Sink

Sink is the output where you locate, molten support to standard fd, file, http and others (continue), on this way,  we can choose where to output trace log.

## Control

Use agent to control our sink type and detail parameter. 

## Report

Report base import info which we do not sampled like error list.


# Contributing

Welcome developers who willing to make it better.

the mail list below you can contract for discuss and improve more power

phobosw@gmail.com

silkcutbeta@gmail.com

You may contribute in the following ways:

* [Repost issues and feedback](https://github.com/chuan-yun/Molten/issues)
* Submit fixes, features via Pull Request

# License

Apache License Version 2.0 see http://www.apache.org/licenses/LICENSE-2.0.html
