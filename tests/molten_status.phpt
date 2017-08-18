--TEST--
molten status check
--INI--
molten.enable=1
molten.tracing_cli=1
molten.sink_type=0
molten.output_type=2
molten.service_name=test
molten.sampling_rate=123456
molten.span_format=zipkin
--FILE--
<?php
include 'config.inc';
include 'server.inc';

$hostname = 'localhost';
$port = '8964';
molten_cli_server_start($hostname, $port, 1);

$url = 'http://' . $hostname . ':' . $port;

function get_status() {
    global $url;
    $ch1 = curl_init($url . '/molten/status');
    curl_setopt($ch1, CURLOPT_RETURNTRANSFER, true);
    $output = curl_exec($ch1);
    echo $output."\n";
}

get_status();

$ch1 = curl_init($url . '/molten/status');
$data = [
    "samplingType" => 2,
    "samplingRate" => 1783,
    "samplingRequest" => 4443,
];
$post = json_encode($data);
curl_setopt($ch1, CURLOPT_RETURNTRANSFER, true);
curl_setopt($ch1, CURLOPT_POST, 1);
curl_setopt($ch1, CURLOPT_POSTFIELDS, $post);
curl_exec($ch1);

get_status();

?>
--EXPECTF--
# HELP molten_request_all Number of all request.
# TYPE molten_request_all counter
molten_request_all %d
# HELP molten_request_capture Number of request be capture.
# TYPE molten_request_capture counter
molten_request_capture %d
# HELP molten_sampling_type the type of sampling.
# TYPE molten_sampling_type gauge
molten_sampling_type %d
# HELP molten_sampling_rate the rate of sampling.
# TYPE molten_sampling_rate gauge
molten_sampling_rate %d
# HELP molten_sampling_request the request be capture one min.
# TYPE molten_sampling_request gauge
molten_sampling_request %d
# HELP molten_version current molten span version.
# TYPE molten_version gauge
molten_version %s

# HELP molten_request_all Number of all request.
# TYPE molten_request_all counter
molten_request_all %d
# HELP molten_request_capture Number of request be capture.
# TYPE molten_request_capture counter
molten_request_capture %d
# HELP molten_sampling_type the type of sampling.
# TYPE molten_sampling_type gauge
molten_sampling_type 2
# HELP molten_sampling_rate the rate of sampling.
# TYPE molten_sampling_rate gauge
molten_sampling_rate %d
# HELP molten_sampling_request the request be capture one min.
# TYPE molten_sampling_request gauge
molten_sampling_request 4443
# HELP molten_version current molten span version.
# TYPE molten_version gauge
molten_version %s
