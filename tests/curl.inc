<?php
include_once 'config.inc';

function truncate_tmp_log()
{
    $fp = fopen(LOG_DIR, "r+");
    ftruncate($fp, 0);
    fclose($fp);
}
?>
