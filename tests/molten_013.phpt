--TEST--
Tracing memcached addServers
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
molten.span_format=zipkin
--FILE--
<?php
include 'config.inc';

/*memcached (exclude: __construct, __destruct, getResultMessage) */
$mem = new Memcached();
$mem->addServers([
   [$true_memcache['host'], $true_memcache['port']],
   [$true_memcache2['host'], $true_memcache2['port']] 
]);
$mem->add("test-monitor-addServers", "the key as long as possible, here we can see the default aroud the other set", 0);
?>
--EXPECTF--
{"traceId":"%s","name":"addServers","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"sa","value":"true","endpoint":{"serviceName":"memcache","ipv4":"%s","port":%d}},{"key":"sa","value":"true","endpoint":{"serviceName":"memcache","ipv4":"%s","port":%d}},{"key":"db.statement","value":"addServers array(2),","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"memcache","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"add","version":"%s","id":"%s","parentId":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"cs","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"cr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"db.statement","value":"add test-monitor-addServers,the key as long as possible, he...","endpoint":{"serviceName":"test","ipv4":"%s"}},{"key":"db.type","value":"memcache","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
{"traceId":"%s","name":"cli","version":"%s","id":"%s","timestamp":%d,"duration":%d,"annotations":[{"value":"sr","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}},{"value":"ss","timestamp":%d,"endpoint":{"serviceName":"test","ipv4":"%s"}}],"binaryAnnotations":[{"key":"path","value":"%s","endpoint":{"serviceName":"test","ipv4":"%s"}}]}
