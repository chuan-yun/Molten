<?php

define('LOG_DIR', __DIR__.'/tmplog');
define('NC_LOG', __DIR__.'/nctmplog');

/* url */
$true_url = "http://1.intra.ffan.com/v1/period/10001";
$fail_url = "http://2.intra.ffan.com/v1/period/10001" ;

/* db */
$true_db = [
    "dsn" => 'mysql:dbname=duobao;host=192.168.56.102;port=3306',
    "host" => '192.168.56.102',
    "user" => 'root',
    "passwd" => 'MyNewPass4!',
    "db" => 'duobao',
];

$fail_db = [
    "dsn" => 'mysql:dbname=duobao;host=192.168.56.102',
    "host" => '192.168.56.112',
    "user" => 'root',
    "passwd" => 'MyNewPass4!',
    "db" => 'duobao',
];

/* redis */
$true_redis = [
    "host" => '10.213.42.125',
    "port" => '10618',
];

$fail_redis = [
    "host" => '10.213.42.125',
    "port" => '10620',
];

/* memcached */
$true_memcache = [
    "host" => '10.209.37.22',
    "port" => '11242',
];

$true_memcache2 = [
    "host" => '10.213.42.12',
    "port" => '11242',
];


$fail_memcache = [
    "host" => '10.209.37.22',
    "port" => '112421',
];

?>
