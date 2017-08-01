<?php
$option = isset($_GET['option']) ? $_GET['option'] : null ;
$header = 'HTTP_X_W_';
$response = '';

if (isset($_GET['trace'])) {
        $response .= 'trace_id:' . $_SERVER[$header . 'TRACEID'] . ',span_id:'. $_SERVER[$header . 'SPANID'] . ',parent_id:' . $_SERVER[$header . 'PARENTSPANID'] . ',sampled:'  . $_SERVER[$header . 'SAMPLED'] . ',flags:'. $_SERVER[$header . 'FLAGS'];
}

if (isset($_GET['reflect'])) {
        $response .= ',query_string:' . $_SERVER['QUERY_STRING'];
}

if (isset($_GET['header'])) {
    $userh = $_GET['header']; 
    $response .= ',header:' . $_SERVER['HTTP_' . strtoupper($userh)];
}


trim($response, ',');
if (isset($_GET['dumplog'])) {
    file_put_contents(LOG_DIR, $response . "\n", FILE_APPEND);
} else {
    echo $response;
}
?>
