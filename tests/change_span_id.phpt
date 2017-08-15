--TEST--
Check span generate rule(1.2.xx)
--SKIPIF--
<?php if ((ini_get("molten.span_format") != "zipkin") || (molten_span_format() == "random")) print "skip"; ?>
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
include 'pdo_execute.inc';
include 'server.inc';

/* execute db */
exec_normal($true_db);

/* mongodb manager */
$manager = new MongoDB\Driver\Manager("mongodb://localhost:27017");
$bulk = new MongoDB\Driver\BulkWrite();
$bulk->insert(['x' => 2]);
$writeConcern = new MongoDB\Driver\WriteConcern(MongoDB\Driver\WriteConcern::MAJORITY, 100);

/* http request */
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

?>
--EXPECTF--
{"traceId":"%s","name":"PDO::exec","version":"%s","id":"1.1","parentId":"1","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"select * from configs","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.instance","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}}]}
{"traceId":"%s","name":"PDO::prepare","version":"%s","id":"1.2","parentId":"1","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"select * from configs","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.instance","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}}]}
{"traceId":"%s","name":"PDOStatement::execute","version":"%s","id":"1.3","parentId":"1","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"select * from configs","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mysql","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.instance","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"sa","value":"true","endpoint":{"serviceName":"mysql","ipv4":"%s","port":%d}}]}
{"traceId":"%s","name":"__construct","version":"%s","id":"1.4","parentId":"1","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"__construct mongodb:\/\/localhost:27017,","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mongodb","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"php.db.source","value":"mongodb:\/\/localhost:27017","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"php_curl","version":"%s","id":"1.5","parentId":"1","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"http:\/\/localhost:8964\/?ch1=1&trace=1&reflect=1","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.response","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"1","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
