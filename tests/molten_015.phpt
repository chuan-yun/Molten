--TEST--
Tracing elasticsearch client
--SKIPIF--
<?php if (ini_get("molten.span_format") != "zipkin") print "skip"; ?>
--INI--
molten.enable=1
molten.tracing_cli=1
molten.sink_type=2
molten.output_type=2
molten.service_name=test
molten.sampling_rate=1
molten.span_format=zipkin
--FILE--
<?php
include 'server.inc';
require "vendor/autoload.php";

use Elasticsearch\ClientBuilder;

$client = ClientBuilder::create()->setHosts($es_host)->build();


 /* create*/
$create = [
    'index' =>  'test',
    'body' =>   [
        'settings'  => [
            'number_of_shards' => 1,
        ]
    ]
];
$client->indices()->create($create);

/* index */
$index = [
    'index' => 'test',
    'type'  => 'test',
    'id'    => 'test_id',
    'body'  => ['testFiled' => '123'],
];

/* get */
$client->index($index);

$get = [
    'index' => 'test',
    'type'  => 'test',
    'id'    => 'test_id',
];

$response = $client->get($get);

?>
--EXPECTF--
{"traceId":"%s","name":"%s","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"create","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"create array(2),","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"componet","value":"Elasticsearch\\Client","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"elasticsearch","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"%s","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"201","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"index","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"index array(4),","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"sa","value":"true","endpoint":{"serviceName":"es","ipv4":"%s","port":%d}},{"key":"db.type","value":"elasticsearch","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"componet","value":"Elasticsearch\\Client","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"%s","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"http.url","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"http.status","value":"200","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"get","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"get array(3),","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"sa","value":"true","endpoint":{"serviceName":"es","ipv4":"%s","port":%d}},{"key":"db.type","value":"elasticsearch","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"componet","value":"Elasticsearch\\Client","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
