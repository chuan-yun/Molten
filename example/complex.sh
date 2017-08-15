#/bin/bash

sh zipkin.sh

export TEST_PHP_EXECUTABLE=`which php`

php -d extension=molten.so -d molten.enable=1 -d molten.sink_type=4 -d molten.tracing_cli=1 -d molten.sink_http_uri=http://127.0.0.1:9411/api/v1/spans -d molten.service_name=complex -d molten.sampling_rate=1 server.php
