--TEST--
Tracing curl_exec error use
--SKIPIF--
<?php if (ini_get("molten.span_format") != zipkin) print "skip"; ?>
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

$hostname = 'localhost';
$port = '8964';
molten_cli_server_start($hostname, $port, 1);

/* test use error address */
$e_path = 'http://' . $hostname . ':' . '8960';
$path = 'http://' . $hostname . ':' . $port;

$url1 = $e_path."?ch1=1&trace=1&reflect=1";
$ch = curl_init($url1);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
curl_setopt($ch, CURLOPT_HTTPHEADER, ["header-fail:test", "header1111111111:222222"]);
$data = curl_exec($ch);
echo $data . "\n";

/* curl error use */
$url2 = $path."?ch2=1&trace=1&reflect=1";
$ch = curl_init($url2);
curl_setopt($ch, CURLOPT_HTTPHEADER, 11111111112121);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$data = curl_exec($ch);
echo $data . "\n";
?>
--EXPECTF--
Warning: molten_curl_setopt(): %s on line 23
trace_id:%s,span_id:%s,parent_id:%s,sampled:1,flags:0,query_string:ch2=1&trace=1&reflect=11
{"traceId":"%s","name":"php_curl","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8960\/?ch1=1&trace=1&reflect=1","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"error","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"php_curl","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8964\/?ch2=1&trace=1&reflect=1","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
