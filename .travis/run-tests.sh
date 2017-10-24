#!/bin/bash
set -ev
MO_PHP_BIN=$1
MO_PHP_VERSION=$2

if [ "$MO_PHP_VERSION"x  == "5.3.27"x ] 
then
    REPORT_EXIT_STATUS=1 TEST_PHP_EXECUTABLE=${MO_PHP_BIN} ${MO_PHP_BIN} -n run-tests.php -q -n -d extension=molten.so -d extension=redis.so -d extension=memcached.so --show-diff
else
    REPORT_EXIT_STATUS=1 TEST_PHP_EXECUTABLE=${MO_PHP_BIN} ${MO_PHP_BIN} -n run-tests.php -q -n -d extension=molten.so -d extension=redis.so -d extension=mongodb.so -d extension=memcached.so --show-diff
fi
