#!/bin/bash
# send  http collector

sh ./zipkin.sh

php -d extension=molten.so -d molten.enable=1 -d molten.sink_type=4 -d molten.tracing_cli=1 -d molten.sink_http_uri=http://127.0.0.1:9411/api/v2/spans -d molten.span_format=zipkin_v2 -d molten.service_name=php_test -d molten.sampling_rate=1 -r '$c=curl_init("http://localhost:12345");curl_exec($c);'
