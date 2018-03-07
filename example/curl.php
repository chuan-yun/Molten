<?php

if (!extension_loaded('molten')) {
	$res = dl('molten.so');
	ini_set("molten.enable","1");
	ini_set("molten.sink_type","6");
	ini_set("molten.tracing_cli","1");
	ini_set("molten.service_name","car-service");
	ini_set("molten.sampling_rate","100");	
}
var_dump(ini_get("molten.enable"));
var_dump(ini_get("molten.sink_type"));
$c=curl_init("http://localhost:12345");
curl_exec($c);
?>
