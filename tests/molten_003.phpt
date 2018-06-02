--TEST--
Tracing PDO error db 
--SKIPIF--
<?php if (ini_get("molten.span_format") != "zipkin") print "skip"; ?>
--INI--
display_errors=Off
molten.enable=1
molten.tracing_cli=1
molten.sink_type=2
molten.output_type=2
molten.service_name=test
molten.sampling_rate=1
molten.open_report=1
--FILE--
<?php
include 'config.inc';
include 'pdo_execute.inc';
exec_normal($fail_db);
exec_error($fail_db);
exec_exception($fail_db);
?>
--EXPECTF--
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"error","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
