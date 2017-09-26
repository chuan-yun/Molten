<?php
$m = new Molten();
$c=curl_init("http://localhost:1231");curl_exec($c);
$header = $m->getTraceHeader("service");
print_r($header);
$header['duration'] = 264778;
$header['binaryAnnotations'] = array(
	array(
			'key' => 'service.connect',
			'value' => '192.168.1.108:9980',
			'endpoint' => array(
					'serviceName' => $header['name'],
					'ipv4' => '127.0.0.1',
					'port' => 80,
			),
	),
	array(
			'key' => 'service.getResult',
			'value' => '192.168.1.108:9980',
			'endpoint' => array(
					'serviceName' => $header['name'],
					'ipv4' => '127.0.0.1',
					'port' => 80,
			),
	),
);
$m->addSpans($header);