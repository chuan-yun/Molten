<?php
function server_start($hostname, $port, $router, $sn) {

    $address = $hostname . ":" . $port;
	$php_executable = getenv('TEST_PHP_EXECUTABLE');
	$doc_root = __DIR__;
    if (empty($router)) {
	    $router = "response.php";
    }

	$descriptorspec = array(
		0 => STDIN,
		1 => STDOUT,
		2 => STDERR,
	);

	if (substr(PHP_OS, 0, 3) == 'WIN') {
		$cmd = "{$php_executable} -t {$doc_root} -n -S " . $address;
        $cmd .= " {$router}";
		$handle = proc_open(addslashes($cmd), $descriptorspec, $pipes, $doc_root, NULL, array("bypass_shell" => true,  "suppress_errors" => true));
	} else {
		$cmd = "exec {$php_executable} -t {$doc_root} -d extension=molten.so -d molten.enable=1 -d molten.sink_type=4 -d molten.span_format=zipkin_v2 -d molten.sink_http_uri=http://127.0.0.1:9411/api/v2/spans -d molten.service_name={$sn} -d molten.sampling_rate_base=1  -S " . $address;
		$cmd .= " {$router}";
		$cmd .= " 2>/dev/null";
		$handle = proc_open($cmd, $descriptorspec, $pipes, $doc_root);
	}
	
    $i = 0;
    while (($i++ < 30) && !($fp = @fsockopen($hostname, $port))) {
        usleep(10000);
    }

    if ($fp) {
        fclose($fp);
    }

	register_shutdown_function(
		function($handle) use($router) {
			proc_terminate($handle);
		},
			$handle
		);
    return $address;
}

server_start("localhost", 8971, "response.php", "base_server");
server_start("localhost", 8972, "http_request.php", "t1_server");
server_start("localhost", 8973, "http_request.php", "t2_server");

$url1 = "http://localhost:8972";
$ch = curl_init($url1);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_POSTFIELDS, 'parameters=' . json_encode("1111"));
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$data = curl_exec($ch);

$url1 = "http://localhost:8973";
$ch = curl_init($url1);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$data = curl_exec($ch);
?>
