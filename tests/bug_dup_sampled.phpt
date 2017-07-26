--TEST--
Bug dup sampled header
--INI--
molten.enable=1
molten.tracing_cli=1
molten.service_name=test
molten.sampling_rate_base=12341234
--FILE--
<?php
include 'config.inc';
include 'server.inc';

$nc_port1 = 8965;
$nc_port2 = 8966;

truncate_file(NC_LOG);
molten_nc_server($nc_port1);
molten_nc_server($nc_port2);

/* request nc server */
$url1 = 'http://localhost:'.$nc_port1."?ch1=1";
$ch = curl_init($url1);
curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch, CURLOPT_NOSIGNAL, 1);
curl_setopt($ch, CURLOPT_POST, 1);
curl_setopt($ch, CURLOPT_POSTFIELDS, 'parameters=' . json_encode("1111"));
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$data = curl_exec($ch);

/* request nc server 2 */
$url2 = 'http://localhost:'.$nc_port2."?ch1=1";
curl_setopt($ch, CURLOPT_URL, $url2);
curl_setopt($ch, CURLOPT_TIMEOUT_MS, 500);
$data = curl_exec($ch);

$content = file_get_contents(NC_LOG);
echo substr_count($content, "X-W-Sampled");
?>
--EXPECTF--
2
