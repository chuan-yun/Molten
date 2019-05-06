--TEST--
Bug for mysqli connect error db >= 7.0.0
--SKIPIF--
<?php if (version_compare(phpversion(), '7.0.0', '<') || (ini_get("molten.span_format") != "zipkin")) print "skip"; ?>
--INI--
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
include 'mysqli_execute.inc';

$sql = 'select * from configs';

/* mysql real_connect */
$mysql = mysqli_init();
$mysql->real_connect($fail_db['host'], $fail_db['user'], $fail_db['passwd'], $fail_db['db']);
$mysql->query($sql);

mysqli_exec($fail_db, $sql);
?>
--EXPECTF--
Warning: mysqli::real_connect(): (HY000/2002): %s on line 9

Warning: mysqli::query(): invalid object or resource mysqli
 in %s on line 10

Warning: mysqli::query(): invalid object or resource mysqli
 in %s on line 10

Warning: mysqli_connect(): (HY000/2002): %s on line 7

Warning: mysqli_query() expects parameter 1 to be mysqli, %s given in %s on line 8

Warning: mysqli_prepare() expects parameter 1 to be mysqli, %s given in %s on line 9

Warning: mysqli_stmt_execute() expects parameter 1 to be mysqli_stmt, null given in %s on line 10

Warning: mysqli::%s(): (HY000/2002): %s on line 17

Warning: mysqli::query(): Couldn't fetch mysqli in %s on line 18

Warning: mysqli::query(): Couldn't fetch mysqli in %s on line 18

Warning: mysqli::query(): Couldn't fetch mysqli in %s on line 18

Warning: mysqli::prepare(): Couldn't fetch mysqli in %s on line 19

Warning: mysqli::prepare(): Couldn't fetch mysqli in %s on line 19

Warning: mysqli::prepare(): Couldn't fetch mysqli in %s on line 19

Warning: mysqli::%s(): (HY000/2002): %s on line 27

Warning: mysqli::stmt_init(): Couldn't fetch mysqli in %s on line 28

%s
%s
%s
%s
%s
{"traceId":"%s","name":"real_connect","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}},{"key":"db.instance","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"error","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"query","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"select * from configs","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"error","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"mysqli_connect","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}},{"key":"db.instance","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"error","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"mysqli_query","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"select * from configs","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"mysqli_prepare","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"select * from configs","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"mysqli_stmt_execute","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"%s","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}},{"key":"db.instance","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"error","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"query","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"select * from configs","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"prepare","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"select * from configs","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"%s","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}},{"key":"db.instance","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"error","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"error","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
