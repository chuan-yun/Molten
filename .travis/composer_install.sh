#!/bin/bash

PHP_VERSION=$1
TEST_PATH=$2

COMPOSER_JSON=$TEST_PATH/composer.json
OLD55_JOSN=$TEST_PATH/composer.json.5.5
OLD54_JOSN=$TEST_PATH/composer.json.5.4
OLD53_JOSN=$TEST_PATH/composer.json.5.3

function version_ge()
{
    python << EOF
import sys
from distutils.version import LooseVersion

sys.exit(not (LooseVersion('$1') > LooseVersion('$2')))
EOF
}

# change es version by php version
if version_ge $PHP_VERSION "5.6.0"; then
    # do nothing
    echo "skip"
elif version_ge $PHP_VERSION "5.5.0";  then
    cp $OLD55_JOSN  $COMPOSER_JSON
elif version_ge $PHP_VERSION "5.4.0";  then
    cp $OLD54_JOSN  $COMPOSER_JSON
else
    cp $OLD53_JOSN  $COMPOSER_JSON
fi
