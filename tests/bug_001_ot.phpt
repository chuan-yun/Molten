--TEST--
Bug for mysqli_real_connect
--SKIPIF--
<?php if (ini_get("molten.span_format") != "opentracing") print "skip"; ?>
--INI--
molten.enable=1
molten.tracing_cli=1
molten.sink_type=2
molten.output_type=2
molten.service_name=test
molten.sampling_rate=1
molten.span_format=opentracing
--FILE--
<?php
include 'config.inc';
include 'pdo_execute.inc';

$sql = 'select * from configs';

/* mysql real_connect */
$mysql = mysqli_init();
$mysql->real_connect($true_db['host'], $true_db['user'], $true_db['passwd'], $true_db['db']);
$mysql->query($sql);

/* mysqli_real_connect */
$mysql = mysqli_init();
mysqli_real_connect($mysql, $true_db['host'], $true_db['user'], $true_db['passwd'], $true_db['db']);
$mysql->query($sql);
?>
--EXPECTF--
{"operationName":"real_connect","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.1","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"query","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.2","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_real_connect","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.3","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"query","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.4","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"cli","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s"},"tags":{"span.kind":"server"},"logs":[]}
