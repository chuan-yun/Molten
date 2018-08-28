--TEST--
Bug dup header
--SKIPIF--
<?php if (ini_get("molten.span_format") != "zipkin") print "skip"; ?>
--INI--
molten.enable=1
molten.tracing_cli=1
molten.sink_type=2
molten.output_type=2
molten.service_name=test
molten.sampling_rate=1
--FILE--
<?php
include 'config.inc';
include 'server.inc';

$hostname = 'localhost';
$port = '8964';
$nc_port = 8965;

truncate_file(NC_LOG);
molten_cli_server_start($hostname, $port, 1);
molten_nc_server($nc_port);

/* normal post */
$path = 'http://' . $hostname . ':' . $port;
$url1 = $path."?ch1=1";
$ch = curl_init($url1);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_POSTFIELDS, 'parameters=' . json_encode("1111"));
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$data = curl_exec($ch);

/* request nc server */
$url2 = 'http://localhost:'.$nc_port."?ch1=1";
curl_setopt($ch, CURLOPT_URL, $url2);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_POSTFIELDS, 'parameters=' . json_encode("1111"));
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$data = curl_exec($ch);
$content = file_get_contents(NC_LOG);
echo substr_count($content, "X-W-TraceId") . "\n" . substr_count($content, "X-W-SpanId") . "\n" . substr_count($content, "X-W-ParentSpanId") . "\n" . substr_count($content, "X-W-Sampled") . "\n". substr_count($content, "X-W-Flags") . "\n";
?>
--EXPECTF--
1
1
1
1
1
{"traceId":"%s","name":"php_curl","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8964\/?ch1=1","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"php_curl","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8965\/?ch1=1","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"error","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"file_get_contents","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"file.path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
