--TEST--
Bug for mysqli_real_connect
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
{"traceId":"%s","name":"real_connect","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}},{"key":"db.instance","value":"duobao","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"query","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"select * from configs","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}},{"key":"db.instance","value":"duobao","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"mysqli_real_connect","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}},{"key":"db.instance","value":"duobao","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"query","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"select * from configs","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}},{"key":"db.instance","value":"duobao","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
