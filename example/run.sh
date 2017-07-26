#!/bin/bash
java=`which java`

if [ -z "$java" ]; then
    echo "can not find java"    
    exit;
fi

if [ ! -f zipkin.jar ]; then
    wget -O zipkin.jar 'https://search.maven.org/remote_content?g=io.zipkin.java&a=zipkin-server&v=LATEST&c=exec'
fi
export STORAGE_TYPE=mem
export HTTP_COLLECTOR_ENABLE=true

zipkin=`ps aux|grep zipkin|grep -v grep`

if [ -z "$zipkin" ]; then
java -jar zipkin.jar --logging.level.zipkin=DEBUG 2>&1 >/dev/null &
fi

# send  http collector
sleep 10
/usr/local/php5.6/bin/php -d molten.enable=1 -d molten.sink_type=4 -d molten.tracing_cli=1 -d molten.sink_http_uri=http://127.0.0.1:9411/api/v1/spans -d molten.span_id_format=random -d molten.service_name=php_test -d molten.sampling_rate_base=1 -r '$c=curl_init("http://localhost:12345");curl_exec($c);'
sleep 5
