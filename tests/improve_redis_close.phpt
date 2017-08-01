--TEST--
Tracing redis
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

$redis = new Redis();
$redis->connect($true_redis['host'], $true_redis['port']);
$redis->set("test-molten", "thisisakeywewillsetitaslongaspossibleissolonglonglonglonglong");
$redis->getHost();
$redis->close();

?>
--EXPECTF--
{"traceId":"%s","name":"connect","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"redis","ipv4":"%s","port":%d}},{"key":"db.statement","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"redis","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"set","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"redis","ipv4":"%s","port":%d}},{"key":"db.statement","value":"set test-molten,thisisakeywewillsetitaslongaspos...,","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"redis","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
