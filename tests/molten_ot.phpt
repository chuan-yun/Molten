--TEST--
Tracing opentracing normal exec
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
include 'pdo_execute.inc';
include 'mysqli_execute.inc';
include 'server.inc';

/* pdo */
exec_normal($true_db);

/* mysqli */
mysqli_exec($true_db, "select * from configs");

/* mysqli error */
mysqli_exec($true_db, "select * from configs_not_exist");

/* set redis*/
$redis = new Redis();
$redis->connect($true_redis['host'], $true_redis['port']);
$redis->set("test-molten", "thisisakeywewillsetitaslongaspossibleissolonglonglonglonglong");
$redis->set(12121222, "thisisakeywewillsetitaslongaspossibleissolonglonglonglonglong");
$redis->getHost();

/* set redis error */
$redis = new Redis();
$redis->connect($fail_redis['host'], $fail_redis['port']);
$redis->set("test-molten");

/* mongodb manager */
$manager = new MongoDB\Driver\Manager("mongodb://localhost:27017");
$bulk = new MongoDB\Driver\BulkWrite();
$bulk->insert(['x' => 2]);
$writeConcern = new MongoDB\Driver\WriteConcern(MongoDB\Driver\WriteConcern::MAJORITY, 100);
$result = $manager->executeBulkWrite('db.collection', $bulk, $writeConcern);

/* mongodb server */
$rp = new MongoDB\Driver\ReadPreference(MongoDB\Driver\ReadPreference::RP_PRIMARY);
$server = $manager->selectServer($rp);
$bulk = new MongoDB\Driver\BulkWrite();
$bulk->insert(['x' => 2000]);
$result = $server->executeBulkWrite("db.collection", $bulk);

/* error */
try {
    $bulk = new MongoDB\Driver\BulkWrite();
    $bulk->insert(['_id'=>1, 'x' => 2000]);
    $bulk->insert(['_id'=>1, 'x' => 3000]);
    $result = $manager->executeBulkWrite("db.collection", $bulk);
} catch(Exception $e) {
}

/* delete */
$bulk = new MongoDB\Driver\BulkWrite();
$bulk->delete(['_id'=>1]);
$result = $manager->executeBulkWrite("db.collection", $bulk);

$hostname = 'localhost';
$port = '8964';
molten_cli_server_start($hostname, $port, 1);

$url = 'http://' . $hostname . ':' . $port;
$ch1 = curl_init($url . '?ch1=1&reflect=1');
$ch2 = curl_init($url . '?ch2=1&reflect=1');
curl_setopt($ch1, CURLOPT_TIMEOUT_MS, 1000);
curl_setopt($ch1, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch2, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch2, CURLOPT_TIMEOUT_MS, 1000);
$mh = curl_multi_init();
curl_multi_add_handle($mh,$ch1);
curl_multi_add_handle($mh,$ch2);

$running=null;
do {
    curl_multi_exec($mh,$running);
} while($running > 0);

//close all the handles
curl_multi_remove_handle($mh, $ch1);
curl_multi_remove_handle($mh, $ch2);
curl_multi_close($mh);

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
echo $data . "\n";

/* normal get with header */
$url2 = $path."?ch2=1&trace=1&reflect=1&header=test321";
$ch = curl_init($url2);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$header = ["test321:testheaderwecansee"];
curl_setopt($ch, CURLOPT_HTTPHEADER, $header);
$data = curl_exec($ch);
echo $data . "\n";

/* normal set opt array */
$url3 = $path."?ch3=1&trace=1&reflect=1&header=testurl2";
$ch = curl_init($url3);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
curl_setopt_array($ch, [
    CURLOPT_RETURNTRANSFER => true,
    CURLOPT_HTTPHEADER => ["testurl2:set_array_omo_as_we_can_see"],
]);
$data = curl_exec($ch);
echo $data . "\n";

/*memcached (exclude: __construct, __destruct, getResultMessage) */
$mem = new Memcached();
$mem->addServers([
   [$true_memcache['host'], $true_memcache['port']],
   [$true_memcache2['host'], $true_memcache2['port']] 
]);
$mem->add("test-monitor-addServers", "the key as long as possible, here we can see the default aroud the other set", 0);
?>
--EXPECTF--
Warning: mysqli_stmt_execute() expects parameter 1 to be mysqli_stmt, boolean given in %s on line 10

Warning: Redis::set() expects at least 2 parameters, 1 given in %s on line 26
trace_id:%s,span_id:1.43,parent_id:1,sampled:1,flags:0,query_string:ch1=1&trace=1&reflect=1
trace_id:%s,span_id:1.44,parent_id:1,sampled:1,flags:0,query_string:ch2=1&trace=1&reflect=1&header=test321,header:testheaderwecansee
trace_id:%s,span_id:1.45,parent_id:1,sampled:1,flags:0,query_string:ch3=1&trace=1&reflect=1&header=testurl2,header:set_array_omo_as_we_can_see
{"operationName":"PDO::exec","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.1","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","db.instance":"%s","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql"},"logs":[]}
{"operationName":"PDO::prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.2","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","db.instance":"%s","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql"},"logs":[]}
{"operationName":"PDOStatement::execute","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.3","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","db.instance":"%s","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql"},"logs":[]}
{"operationName":"mysqli_connect","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.4","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_query","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.5","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.6","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_stmt_execute","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.7","parentSpanID":"1"},"tags":{"span.kind":"client","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"%s","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.8","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"query","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.9","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.10","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"execute","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.11","parentSpanID":"1"},"tags":{"span.kind":"client","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"%s","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.12","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.13","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"execute","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.14","parentSpanID":"1"},"tags":{"span.kind":"client","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"%s","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.15","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_stmt_prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.16","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_stmt_execute","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.17","parentSpanID":"1"},"tags":{"span.kind":"client","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_connect","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.18","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_query","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.19","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs_not_exist","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.20","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs_not_exist","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_stmt_execute","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.21","parentSpanID":"1"},"tags":{"span.kind":"client","db.type":"mysql"},"logs":[]}
{"operationName":"%s","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.22","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"query","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.23","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs_not_exist","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"Table 'duobao.configs_not_exist' doesn't exist"}}]}
{"operationName":"prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.24","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs_not_exist","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"Table 'duobao.configs_not_exist' doesn't exist"}}]}
{"operationName":"%s","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.25","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.26","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs_not_exist","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"Table 'duobao.configs_not_exist' doesn't exist"}}]}
{"operationName":"%s","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.27","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"mysqli_stmt_prepare","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.28","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"select * from configs_not_exist","db.type":"mysql","peer.ipv4":"%s","peer.port":%d,"peer.service":"mysql","db.instance":"%s"},"logs":[]}
{"operationName":"connect","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.29","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":10618,"peer.service":"redis","db.statement":"connect 10.213.42.125,10618,","db.type":"redis"},"logs":[]}
{"operationName":"set","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.30","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":10618,"peer.service":"redis","db.statement":"set test-molten,thisisakeywewillsetitaslongaspos...,","db.type":"redis"},"logs":[]}
{"operationName":"set","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.31","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":10618,"peer.service":"redis","db.statement":"set 12121222,thisisakeywewillsetitaslongaspos...,","db.type":"redis"},"logs":[]}
{"operationName":"connect","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.32","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":10620,"peer.service":"redis","db.statement":"connect 10.213.42.125,10620,","db.type":"redis"},"logs":[]}
{"operationName":"set","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.33","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":10620,"peer.service":"redis","db.statement":"set test-molten,","db.type":"redis"},"logs":[]}
{"operationName":"__construct","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.34","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"__construct mongodb:\/\/localhost:27017,","db.type":"mongodb","peer.address":"mongodb:\/\/localhost:27017"},"logs":[]}
{"operationName":"executeBulkWrite","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.35","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"executeBulkWrite db.collection,object(MongoDB\\Driver\\BulkWr...","db.type":"mongodb","peer.address":"mongodb:\/\/localhost:27017"},"logs":[]}
{"operationName":"executeBulkWrite","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.36","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"executeBulkWrite db.collection,object(MongoDB\\Driver\\BulkWr...","db.type":"mongodb","peer.ipv4":"localhost","peer.port":27017,"peer.service":"mongodb"},"logs":[]}
{"operationName":"executeBulkWrite","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.37","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"executeBulkWrite db.collection,object(MongoDB\\Driver\\BulkWr...","db.type":"mongodb","peer.address":"mongodb:\/\/localhost:27017","peer.ipv4":"%s","peer.port":0,"peer.service":"test"},"logs":[]}
{"operationName":"executeBulkWrite","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.38","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"executeBulkWrite db.collection,object(MongoDB\\Driver\\BulkWr...","db.type":"mongodb","peer.address":"mongodb:\/\/localhost:27017"},"logs":[]}
{"operationName":"php_curl_multi","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.41","parentSpanID":"1"},"tags":{"span.kind":"client","http.url":"http:\/\/localhost:8964\/?ch1=1&reflect=1","http.status":"200"},"logs":[]}
{"operationName":"php_curl_multi","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.42","parentSpanID":"1"},"tags":{"span.kind":"client","http.url":"http:\/\/localhost:8964\/?ch2=1&reflect=1","http.status":"200"},"logs":[]}
{"operationName":"php_curl","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.43","parentSpanID":"1"},"tags":{"span.kind":"client","http.url":"http:\/\/localhost:8964\/?ch1=1&trace=1&reflect=1","http.status":"200","http.response":"%s"},"logs":[]}
{"operationName":"php_curl","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.44","parentSpanID":"1"},"tags":{"span.kind":"client","http.url":"http:\/\/localhost:8964\/?ch2=1&trace=1&reflect=1&header=test321","http.status":"200","http.response":"%s"},"logs":[]}
{"operationName":"php_curl","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.45","parentSpanID":"1"},"tags":{"span.kind":"client","http.url":"http:\/\/localhost:8964\/?ch3=1&trace=1&reflect=1&header=testurl2","http.status":"200","http.response":"%s"},"logs":[]}
{"operationName":"addServers","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.46","parentSpanID":"1"},"tags":{"span.kind":"client","peer.ipv4":"%s","peer.port":11242,"peer.service":"memcache","db.statement":"addServers array(2),","db.type":"memcache"},"logs":[]}
{"operationName":"add","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s.47","parentSpanID":"1"},"tags":{"span.kind":"client","db.statement":"add test-monitor-addServers,the key as long as possible, he...","db.type":"memcache","error":true},"logs":[{"timestamp":%d,"fields":{"event":"error","error.kind":"Exception","message":"NOT STORED"}}]}
{"operationName":"cli","startTime":%d,"finishTime":%d,"spanContext":{"traceID":"%s","spanID":"%s"},"tags":{"span.kind":"server"},"logs":[]}
