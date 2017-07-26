--TEST--
Bug for mysqli connect error db < 7.0.0
--SKIPIF--
<?php if (version_compare(phpversion(), '7.0.0', '>=') || (ini_get("molten.span_format") != "opentracing")) print "skip"; ?>
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
include 'mysqli_execute.inc';

$sql = 'select * from configs';

/* mysql real_connect */
$mysql = mysqli_init();
$mysql->real_connect($fail_db['host'], $fail_db['user'], $fail_db['passwd'], $fail_db['db']);
$mysql->query($sql);

mysqli_exec($fail_db, $sql);
?>
--EXPECTF--
Warning: mysqli::real_connect(): (HY000/2002): No route to host in %s on line 9

Warning: mysqli::query(): invalid object or resource mysqli
 in %s on line 10

Warning: mysqli::query(): invalid object or resource mysqli
 in %s on line 10

Warning: mysqli_connect(): (HY000/2002): No route to host in %s on line 7

Warning: mysqli_query() expects parameter 1 to be mysqli, boolean given in %s on line 8

Warning: mysqli_prepare() expects parameter 1 to be mysqli, boolean given in %s on line 9

Warning: mysqli_stmt_execute() expects parameter 1 to be mysqli_stmt, null given in %s on line 10

Warning: mysqli::%s(): (HY000/2002): No route to host in %s on line 17

Warning: mysqli::query(): Couldn't fetch mysqli in %s on line 18

Warning: mysqli::query(): Couldn't fetch mysqli in %s on line 18

Warning: mysqli::query(): Couldn't fetch mysqli in %s on line 18

Warning: mysqli::prepare(): Couldn't fetch mysqli in %s on line 19

Warning: mysqli::prepare(): Couldn't fetch mysqli in %s on line 19

Warning: mysqli::prepare(): Couldn't fetch mysqli in %s on line 19

Warning: mysqli::%s(): (HY000/2002): No route to host in %s on line 27

Warning: mysqli::stmt_init(): Couldn't fetch mysqli in %s on line 28

%s
{"operationName":"real_connect","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.1","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"No route to host"}}]}
{"operationName":"query","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.2","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"No route to host"}}]}
{"operationName":"mysqli_connect","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.3","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"No route to host"}}]}
{"operationName":"mysqli_query","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.4","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql"},"logs":[]}
{"operationName":"mysqli_prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.5","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql"},"logs":[]}
{"operationName":"mysqli_stmt_execute","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.6","parentSpanID":"1"},"tags":{"span.kind":"client","db.type":"mysql"},"logs":[]}
{"operationName":"mysqli","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.7","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"No route to host"}}]}
{"operationName":"query","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.8","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql"},"logs":[]}
{"operationName":"prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.9","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql"},"logs":[]}
{"operationName":"mysqli","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.10","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"No route to host"}}]}
{"operationName":"cli","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s"},"tags":{"span.kind":"server","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"%s"}}]}
