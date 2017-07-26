--TEST--
Tracing redis
--SKIPIF--
<?php if (ini_get("molten.span_format") != "zipkin") print "skip"; ?>
--INI--
molten.enable=1
molten.tracing_cli=1
molten.service_name=test
molten.sink_type=2
molten.output_type=2
molten.sampling_rate_base=1
molten.span_format=zipkin
--FILE--
<?php
include 'config.inc';

/* set true */
$redis = new Redis();
$redis->connect($true_redis['host'], $true_redis['port']);
$redis->set("test-molten", "thisisakeywewillsetitaslongaspossibleissolonglonglonglonglong");
$redis->set(12121222, "thisisakeywewillsetitaslongaspossibleissolonglonglonglonglong");
$redis->getHost();

/* set error */
$redis = new Redis();
$redis->connect($fail_redis['host'], $fail_redis['port']);
$redis->set("test-molten");

?>
--EXPECTF--
Warning: Redis::set() expects at least 2 parameters, 1 given in %s on line 14
{"traceId":"%s","name":"connect","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"redis","ipv4":"%s","port":10618}},{"key":"db.statement","value":"connect 10.213.42.125,10618,","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"redis","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"set","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"redis","ipv4":"%s","port":10618}},{"key":"db.statement","value":"set test-molten,thisisakeywewillsetitaslongaspos...,","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"redis","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"set","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"redis","ipv4":"%s","port":10618}},{"key":"db.statement","value":"set 12121222,thisisakeywewillsetitaslongaspos...,","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"redis","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"connect","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"redis","ipv4":"%s","port":10620}},{"key":"db.statement","value":"connect 10.213.42.125,10620,","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"redis","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"set","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"redis","ipv4":"%s","port":10620}},{"key":"db.statement","value":"set test-molten,","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"redis","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
