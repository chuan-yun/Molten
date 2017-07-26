--TEST--
Tracing curl_exec normal
--SKIPIF--
<?php if (ini_get("molten.span_format") != "zipkin") print "skip"; ?>
--INI--
molten.enable=1
molten.tracing_cli=1
molten.sink_type=2
molten.output_type=2
molten.service_name=test
molten.sampling_rate_base=1
molten.span_format=zipkin
--FILE--
<?php
include 'server.inc';

$hostname = 'localhost';
$port = '8964';
molten_cli_server_start($hostname, $port, 1);

$path = 'http://' . $hostname . ':' . $port;

/* normal post */
$url1 = $path."?ch1=1&trace=1&reflect=1";
$ch = curl_init($url1);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_POSTFIELDS, 'parameters=' . json_encode("1111"));
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$data = curl_exec($ch);
echo $data . "\n";

/* normal get with header */
$url2 = $path."?ch2=1&trace=1&reflect=1&header=test321";
$ch = curl_init($url2);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$header = ["test321:testheaderwecansee"];
curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
$data = curl_exec($ch);
echo $data . "\n";

/* normal set opt array */
$url3 = $path."?ch3=1&trace=1&reflect=1&header=testurl2";
$ch = curl_init($url3);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
curl_setopt_array($ch, [
    CURLOPT_RETURNTRANSFER => true,
    CURLOPT_HTTPHEADER => ["testurl2:set_array_omo_as_we_can_see"],
]);
$data = curl_exec($ch);
echo $data . "\n";

?>
--EXPECTF--
trace_id:%s,span_id:%s,parent_id:%s,sampled:1,flags:0,query_string:ch1=1&trace=1&reflect=1
trace_id:%s,span_id:%s,parent_id:%s,sampled:1,flags:0,query_string:ch2=1&trace=1&reflect=1&header=test321,header:testheaderwecansee
trace_id:%s,span_id:%s,parent_id:%s,sampled:1,flags:0,query_string:ch3=1&trace=1&reflect=1&header=testurl2,header:set_array_omo_as_we_can_see
{"traceId":"%s","name":"php_curl","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8964\/?ch1=1&trace=1&reflect=1","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.response","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"php_curl","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8964\/?ch2=1&trace=1&reflect=1&header=test321","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.response","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"php_curl","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8964\/?ch3=1&trace=1&reflect=1&header=testurl2","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.response","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
