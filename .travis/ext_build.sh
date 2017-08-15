#!/bin/bash
function logit() {
    echo "[ext_build] $@" 1>&2
}

function change_ex_name()
{
    new_name=$1
    extension_dir=`$phpcfg --extension-dir`
    cp $extension_dir/molten.so $extension_dir/${new_name}.so
}

function build()
{
    php_path=`cd $1; pwd`
    extension_name=$2

    php="$php_path/bin/php"
    phpize="$php_path/bin/phpize"
    phpcfg="$php_path/bin/php-config"
    if [ ! -f $phpcfg ]; then
        logit "invalid PHP path $php_path"
        return 1
    fi

    # clean
    make clean      >/dev/null 2>&1
    $phpize --clean >/dev/null 2>&1

    # configure, make
    $phpize &&
    if [ -z "$extension_name" ]; then
        ./configure --with-php-config=$phpcfg && \
        make install
    else
        ./configure --with-php-config=$phpcfg --enable-level-id && \
        make install
        change_ex_name $extension_name
    fi

    ret=$?

    if [ $ret -eq 0 ]; then
        logit "done"
    else
        logit "fail"
    fi

    return $ret
}

# main
if [ $# -lt 1 ]; then
    echo "usage: `basename $0` <php-path>"
    exit 1
fi

# argument
php_path="$1"
if [ ! -d "$php_path" ]; then
    logit "error: invalid PHP path \"$php_path\""
    exit 1
fi
logit "php_path: $php_path"

# build
build $php_path $2
