<?php
//$hostname = isset($argv[1]) ? $argv[1] : 'localhost';
//$port = isset($argv[2]) ? $argv[2] : 8964;
//$address = $hostname . ":" . $port;
//$sample = isset($argv[3]);
include_once 'config.inc';

function truncate_file($path)
{
    if (file_exists($path)) {
        $fp = fopen($path, "r+");
        ftruncate($fp, 0);
        fclose($fp);
    }
}

function molten_nc_server($port) {
    $host = 'localhost';
    $address =  $host . ":" . $port;

	$descriptorspec = array(
		0 => STDIN,
		1 => STDOUT,
		2 => STDERR,
	);
	$doc_root = __DIR__;

    $cmd = "exec netcat -kl -p $port -c 'xargs -n1 echo >>" . NC_LOG .  "'";
	$cmd .= " 2>/dev/null";
	$handle = proc_open($cmd, $descriptorspec, $pipes, $doc_root);
    sleep(1);
	register_shutdown_function(
		function($handle) {
			proc_terminate($handle);
		},
			$handle
		);
}

function molten_cli_server_start($hostname, $port, $sample) {

    $address = $hostname . ":" . $port;
	$php_executable = getenv('TEST_PHP_EXECUTABLE');
	$doc_root = __DIR__;
	$router = "response.inc";

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
		$cmd = "exec {$php_executable} -t {$doc_root} -n -d extension={$doc_root}/../modules/molten.so -d molten.enable=1 -d molten.sampling_rate_base={$sample} -S " . $address;
		$cmd .= " {$router}";
		$cmd .= " 2>/dev/null";
		$handle = proc_open($cmd, $descriptorspec, $pipes, $doc_root);
	}
	
	// note: even when server prints 'Listening on localhost:8964...Press Ctrl-C to quit.'
	//       it might not be listening yet...need to wait until fsockopen() call returns
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
	// don't bother sleeping, server is already up
	// server can take a variable amount of time to be up, so just sleeping a guessed amount of time
	// does not work. this is why tests sometimes pass and sometimes fail. to get a reliable pass
	// sleeping doesn't work.
    return $address;
}

//molten_cli_server_start("localhost", '8967', 1);
//sleep(10000);
