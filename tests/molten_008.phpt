--TEST--
Tracing mongodb
--SKIPIF--
<?php if (ini_get("molten.span_format") != "zipkin") print "skip"; ?>
--INI--
molten.enable=1
molten.tracing_cli=1
molten.service_name=test
molten.sink_type=2
molten.output_type=2
molten.sampling_rate=1
molten.span_format=zipkin
--FILE--
<?php
include 'config.inc';

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

?>
--EXPECTF--
{"traceId":"%s","name":"__construct","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"__construct mongodb:\/\/localhost:27017,","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mongodb","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"php.db.source","value":"mongodb:\/\/localhost:27017","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"executeBulkWrite","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"executeBulkWrite db.collection,object(MongoDB\\Driver\\BulkWr...","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mongodb","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"php.db.source","value":"mongodb:\/\/localhost:27017","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"executeBulkWrite","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"executeBulkWrite db.collection,object(MongoDB\\Driver\\BulkWr...","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mongodb","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"sa","value":"true","endpoint":{"serviceName":"mongodb","ipv4":"localhost","port":27017}}]}
{"traceId":"%s","name":"executeBulkWrite","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"executeBulkWrite db.collection,object(MongoDB\\Driver\\BulkWr...","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mongodb","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"php.db.source","value":"mongodb:\/\/localhost:27017","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"error","value":"E11000 duplicate key error collection: db.collection index: _id_ dup key: { : 1 }","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"executeBulkWrite","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"executeBulkWrite db.collection,object(MongoDB\\Driver\\BulkWr...","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"mongodb","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"php.db.source","value":"mongodb:\/\/localhost:27017","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
