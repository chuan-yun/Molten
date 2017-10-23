<?php

define('LOG_DIR', __DIR__.'/tmplog');
define('NC_LOG', __DIR__.'/nctmplog');

/* db */
$true_db = array(
    "dsn" => 'mysql:dbname=molten;host=127.0.0.1;port=3306',
    "host" => '127.0.0.1',
    "user" => 'root',
    "passwd" => '',
    "db" => 'molten',
);

$fail_db = array(
    "dsn" => 'mysql:dbname=molten;host=127.0.0.2',
    "host" => '127.0.0.2',
    "user" => 'root',
    "passwd" => 'travis',
    "db" => 'molten',
);

/* redis */
$true_redis = array(
    "host" => '127.0.0.1',
    "port" => '6380',
);

$fail_redis = array(
    "host" => '127.0.0.1',
    "port" => '6389',
);

/* memcached */
$true_memcache = array(
    "host" => '127.0.0.1',
    "port" => '11211',
);

$true_memcache2 = array(
    "host" => '127.0.0.1',
    "port" => '11211',
);


$fail_memcache = array(
    "host" => '127.0.0.1',
    "port" => '11222',
);

/* es */
$es_host = array(
    '127.0.0.1:9200'
);

?>
