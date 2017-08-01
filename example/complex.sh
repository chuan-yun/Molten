#/bin/bash
if [ ! -f zipkin.jar ]; then
    wget -O zipkin.jar 'https://search.maven.org/remote_content?g=io.zipkin.java&a=zipkin-server&v=LATEST&c=exec'
fi
export STORAGE_TYPE=mem
export HTTP_COLLECTOR_ENABLE=true

zipkin=`ps aux|grep zipkin|grep -v grep`

if [ -z "$zipkin" ]; then
java -jar zipkin.jar --logging.level.zipkin=DEBUG 2>&1 >/dev/null &
sleep 10
fi

export TEST_PHP_EXECUTABLE=`which php`

php -d extension=molten.so -d molten.enable=1 -d molten.sink_type=4 -d molten.tracing_cli=1 -d molten.sink_http_uri=http://127.0.0.1:9411/api/v1/spans -d molten.span_id_format=random -d molten.service_name=complex -d molten.sampling_rate=1 server.php
