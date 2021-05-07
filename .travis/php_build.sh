#!/bin/bash
function logit() {
    echo "[php_build] $@" 1>&2
}

function build_from_tar()
{
    tar="$2"
    if [ `uname -s` = "Darwin" ]; then
        src=`echo $tar | sed -E 's/^.*\/?(php-[0-9]+\.[0-9]+\.[0-9]+)\.tar\.bz2$/\1/'`
    else
        src=`echo $tar | sed 's/^.*\/\?\(php-[0-9]\+\.[0-9]\+\.[0-9]\+\)\.tar\.bz2$/\1/'`
    fi
    if [ -z "$src" ]; then
        return 1
    fi

    # prepare normal
    logit "extract tar ball"
    rm -fr $src && tar jxf $tar

    # build
    build $1 $src
}

function build()
{
    # init
    prefix=$1
    src=$2

    # debug
    param_debug=""
    prefix_debug=""
    if [ -n "$3" ]; then
        param_debug="--enable-debug"
        prefix_debug="-debug"
    fi

    # version related
    if [ `uname -s` = "Darwin" ]; then
        version=`grep ' PHP_VERSION ' $src/main/php_version.h | sed -E 's/^#define PHP_VERSION "([0-9a-zA-Z\.]+)".*$/\1/'`
    else
        version=`grep ' PHP_VERSION ' $src/main/php_version.h | sed 's/^#define PHP_VERSION "\([0-9a-zA-Z\.]\+\)".*$/\1/'`
    fi
    buildname="php-${version}${prefix_debug}"
    logit "[$buildname] build"

    # patch
    selfpath=`dirname $0`
    if [ -f $selfpath/php-${version}.patch ]; then
        cp $selfpath/php-${version}.patch $src/trace.patch
    fi
    cd $src
    if [ -f trace.patch ]; then
        patch -p0 --verbose < trace.patch
        logit "[$buildname] patch"
    fi

    # prepare
    param_general="--disable-all $param_debug"
    param_sapi="--enable-cli --enable-cgi"
    if [ ${version:0:3} == "7.3" ]; then
        param_ext="--without-libzip"
    # hack for PHP 5.2
    elif [ ${version:0:3} == "5.2" ]; then
        # pcre: run-tests.php
        # spl:  spl_autoload_register in trace's tests
        param_ext="--with-pcre-regex --enable-spl"
    else
        param_ext=""
    fi

    param_ext="$param_ext --with-bz2 --enable-sockets --with-curl --with-zlib --enable-zip --enable-json --enable-session --enable-pdo --with-mysql=mysqlnd --with-mysqli=mysqlnd --with-pdo-mysql=mysqlnd --with-pear --enable-xml --enable-libxml --enable-phar --enable-filter --enable-hash --with-iconv --with-openssl"
    cmd="./configure --quiet --prefix=$prefix/$buildname $param_general $param_sapi $param_ext"

    # configure
    logit "[$buildname] configure"
    logit "$cmd"
    $cmd

    # make and install
    logit "[$buildname] make"
    # NOT DO a meaningless "make clean"! it's just extracted
    make --quiet && \
    make install
    ret=$?

    if [ $ret -eq 0 ]; then
        logit "[$buildname] done"
    else
        logit "[$buildname] fail"
    fi
}

# main
if [ $# -lt 2 ]; then
    echo "usage: `basename $0` <prefix> <php-tarfile>"
    exit 1
fi

# argument
prefix="$1"
if [ ! -d "$prefix" ]; then
    logit "error: invalid prefix \"$prefix\""
    exit 1
fi
logit "prefix: $prefix"
tarfile="$2"
if [ ! -f "$tarfile" ]; then
    logit "error: invalid PHP tar file \"$tarfile\""
    exit 1
fi
logit "tarfile: $tarfile"

# build
build_from_tar $prefix $tarfile
