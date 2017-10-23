#!/bin/bash

version_ge()
{
    python << EOF
import sys
from distutils.version import LooseVersion

sys.exit(not (LooseVersion('$1') > LooseVersion('$2')))
EOF
}

PHP_PATH=$1
PHP_VERSION=$2

PHP_BIN=$PHP_PATH/bin/php
PHP_PECL=$PHP_PATH/bin/pecl
PHP_IZE=$PHP_PATH/bin/phpize
PHP_CONF=$PHP_PATH/bin/php-config

MO_EX_DIR=`$PHP_BIN -i|grep extension_dir|grep debug|awk -F'=>' '{print $2}'`

ls -a $MO_EX_DIR |grep redis || printf "\n" | $PHP_PECL install -f --ignore-errors redis

# mongodb not support php 5.3
if version_ge $PHP_VERSION "5.4.0"; then
    ls -a $MO_EX_DIR |grep mongodb || printf "\n" | $PHP_PECL install -f --ignore-errors mongodb
fi

PHP_MAJOR=`echo $PHP_VERSION|cut -d'.' -f1`

if [ "$PHP_MAJOR" -eq "7" ] ;then
#ls -a $MO_EX_DIR |grep memcached || printf "\n" | $PHP_PECL install -f --ignore-errors memcached-3.0.2
ls -a $MO_EX_DIR |grep memcached || (wget http://pecl.php.net/get/memcached-3.0.2.tgz && tar xf memcached-3.0.2.tgz && cd memcached-3.0.2 && $PHP_IZE && ./configure --with-php-config=$PHP_CONF --disable-memcached-sasl && make && make install)
else
#ls -a $MO_EX_DIR |grep memcached || printf "\n" | $PHP_PECL install -f --ignore-errors memcached-2.2.0
ls -a $MO_EX_DIR |grep 1memcached || (wget http://pecl.php.net/get/memcached-2.2.0.tgz && tar xf memcached-2.2.0.tgz && cd memcached-2.2.0 && $PHP_IZE && ./configure --with-php-config=$PHP_CONF --disable-memcached-sasl && make && make install)
fi
