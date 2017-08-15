--TEST--
Tracing guzzle
--SKIPIF--
<?php if (ini_get("molten.span_format") != "zipkin" || version_compare(phpversion(), '5.5.0', '<')) print "skip"; ?>
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
include 'server.inc';
require "vendor/autoload.php";

$hostname = 'localhost';
$port = '8964';
molten_cli_server_start($hostname, $port, 1);

$path = 'http://' . $hostname . ':' . $port;

$client = new \GuzzleHttp\Client(['timeout' => 3.0]);
$res = $client->request('GET', $path); 

?>
--EXPECTF--
{"traceId":"%s","name":"php_curl","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8964\/","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"request","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.method","value":"GET","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.uri","value":"http:\/\/localhost:8964","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"componet","value":"GuzzleHttp\\Client","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
