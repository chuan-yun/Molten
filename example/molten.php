<?php
// $m = new Molten();

// $header = $m->getTraceHeader("service");
// $header['duration'] = 264778;
// $header['binaryAnnotations'] = array(
// 	array(
// 			'key' => 'service.connect',
// 			'value' => '192.168.1.108:9980',
// 			'endpoint' => array(
// 					'serviceName' => $header['name'],
// 					'ipv4' => '127.0.0.1',
// 					'port' => 80,
// 			),
// 	),
// 	array(
// 			'key' => 'service.getResult',
// 			'value' => '192.168.1.108:9980',
// 			'endpoint' => array(
// 					'serviceName' => $header['name'],
// 					'ipv4' => '127.0.0.1',
// 					'port' => 80,
// 			),
// 	),
// );
// $m->addSpans($header);
// $m->on("Flush",function($data){
// 	print_r($data);
// });
// $c=curl_init("http://localhost:1231");curl_exec($c);
// $m->flush();
// $c=curl_init("http://local.stats.chelun.com/test/index");curl_exec($c);

class Report
{
	public $client;
	public $mo;
	const EOF = "\r\n";
	
	function __construct()
	{		
		$client = new \swoole_client(SWOOLE_SOCK_TCP);
		$client->set(array('open_eof_check' => true, 'package_eof' => self::EOF));
		$res = $client->connect("127.0.0.1", 9503);
		if ($res) {
			echo "connect success\n";
			$this->client = $client;
			$this->client->send("connect success\n");
		} else {
			exit("connect error");
		}
		
		$m = new Molten();
		$m->on("Flush",[$this,"onFlush"]);
		$this->mo = $m;		
	}
	
	function onFlush($data)
	{		
		$this->client->send(json_encode($data));
	}	
	
	function run()
	{
		$header = $this->mo->getTraceHeader("service");
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
		$this->mo->addSpans($header);
		$c=curl_init("http://localhost:1231");curl_exec($c);
		$c=curl_init("http://localhost:80");curl_exec($c);
		$this->mo->Flush();
	}
}

$r = new Report();
$r->run();

