<?php
require_once dirname(dirname(__FILE__)) . '/src/HelloWorld.php';

molten_set_service_name('php_demo_index');

$o = new HelloWorld();
echo $o->curl("http://192.168.60.7:10400/remote.php");

phpinfo();