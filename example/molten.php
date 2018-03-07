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
// 		$client = new \swoole_client(SWOOLE_SOCK_TCP);
// 		$client->set(array('open_eof_check' => true, 'package_eof' => self::EOF));
		$client = new \swoole_client(SWOOLE_SOCK_TCP);
		$client->set([
				'open_length_check' => true,
				'package_length_type' => 'N',
				'package_length_offset' => 0,       //第N个字节是包长度的值
				'package_body_offset' => 4,       //第几个字节开始计算长度
				'package_max_length' => 2000000,  //协议最大长度
		]);
		$res = $client->connect("127.0.0.1", 9981);
		if ($res) {
			echo "connect success\n";
			$this->client = $client;
		} else {
			exit("connect error");
		}
		
		$m = new Molten();
		$m->on("Flush",[$this,"onFlush"]);
		$this->mo = $m;		
	}
	
	function onFlush($data)
	{		
		$_send_data= json_encode($data);
		$send = pack('N', strlen($_send_data)) . $_send_data;
		$this->client->send($send);
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

