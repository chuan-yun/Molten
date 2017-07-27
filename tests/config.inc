<?php

define('LOG_DIR', __DIR__.'/tmplog');
define('NC_LOG', __DIR__.'/nctmplog');

/* db */
$true_db = [
    "dsn" => 'mysql:dbname=molten;host=127.0.0.1;port=3306',
    "host" => '127.0.0.1',
    "user" => 'root',
    "passwd" => '',
    "db" => 'molten',
];

$fail_db = [
    "dsn" => 'mysql:dbname=molten;host=127.0.0.2',
    "host" => '127.0.0.2',
    "user" => 'root',
    "passwd" => 'travis',
    "db" => 'molten',
];

/* redis */
$true_redis = [
    "host" => '127.0.0.1',
    "port" => '6380',
];

$fail_redis = [
    "host" => '127.0.0.1',
    "port" => '6389',
];

/* memcached */
$true_memcache = [
    "host" => '127.0.0.1',
    "port" => '11211',
];

$true_memcache2 = [
    "host" => '127.0.0.1',
    "port" => '11211',
];


$fail_memcache = [
    "host" => '127.0.0.1',
    "port" => '11222',
];

?>
