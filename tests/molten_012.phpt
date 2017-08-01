--TEST--
Tracing tracing info pass
--INI--
molten.enable=0
molten.tracing_cli=1
molten.sink_type=2
molten.output_type=2
molten.service_name=test
molten.sampling_rate=1
--FILE--
<?php
include 'server.inc';
include 'curl.inc';

//truncate_tmp_log();
//
//$hostname = 'localhost';
//$port = '8964';
//molten_cli_server_start($hostname, $port, 1);
//$path = 'http://' . $hostname . ':' . $port;
//$url1 = $path."?ch1=1&trace=1&reflect=1&dumplog=1";
//$ch = curl_init($url1);
//$trace_id = 'trace_id_test';
//$span_id = 'span_id_test';
//$parent_span_id = 'parent_span_id_test';
//
//curl_setopt($ch, CURLOPT_HTTPHEADER, [
//    "x-w-sampled: 1",
//    "x-w-traceid: $trace_id",
//    "x-w-spanid: $span_id",
//    "x-w-parentspanid: $parent_span_id",
//]);
//curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
//$data = curl_exec($ch);
//echo $data . "\n";

?>
--EXPECTF--
