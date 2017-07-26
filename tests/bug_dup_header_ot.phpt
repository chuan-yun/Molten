--TEST--
Bug dup header
--SKIPIF--
<?php if (ini_get("molten.span_format") != "opentracing") print "skip"; ?>
--INI--
molten.enable=1
molten.tracing_cli=1
molten.sink_type=2
molten.output_type=2
molten.service_name=test
molten.sampling_rate_base=1
molten.span_format=opentracing
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
{"operationName":"php_curl","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.1","parentSpanID":"1"},"tags":{"span.kind":"client","http.url":"http:\/\/localhost:8964\/?ch1=1","http.status":"200"},"logs":[]}
{"operationName":"php_curl","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.2","parentSpanID":"1"},"tags":{"span.kind":"client","http.url":"http:\/\/localhost:8965\/?ch1=1","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"%s"}}]}
{"operationName":"cli","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s"},"tags":{"span.kind":"server"},"logs":[]}
