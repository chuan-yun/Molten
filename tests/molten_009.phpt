--TEST--
Tracing curl_multi_exec
--SKIPIF--
<?php if (ini_get("molten.span_format") != "zipkin") print "skip"; ?>
--INI--
molten.enable=1
molten.tracing_cli=1
molten.sink_type=2
molten.output_type=2
molten.service_name=test
molten.sampling_rate=1
molten.span_format=zipkin
--FILE--
<?php
include 'config.inc';
include 'server.inc';

$hostname = 'localhost';
$port = '8964';
molten_cli_server_start($hostname, $port, 1);

$url = 'http://' . $hostname . ':' . $port;
$ch1 = curl_init($url . '?ch1=1&reflect=1');
$ch2 = curl_init($url . '?ch2=1&reflect=1');
curl_setopt($ch1, CURLOPT_TIMEOUT_MS, 1000);
curl_setopt($ch1, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch2, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch2, CURLOPT_TIMEOUT_MS, 1000);
$mh = curl_multi_init();
curl_multi_add_handle($mh,$ch1);
curl_multi_add_handle($mh,$ch2);

$running=null;
do {
    curl_multi_exec($mh,$running);
} while($running > 0);

//close all the handles
curl_multi_remove_handle($mh, $ch1);
curl_multi_remove_handle($mh, $ch2);
curl_multi_close($mh);

?>
--EXPECTF--
{"traceId":"%s","name":"php_curl_multi","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8964\/?ch1=1&reflect=1","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"php_curl_multi","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8964\/?ch2=1&reflect=1","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
