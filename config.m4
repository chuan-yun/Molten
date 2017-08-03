PHP_ARG_ENABLE(molten, whether to enable molten support,
[  --enable-molten           Enable molten support])

if test "$PHP_PRACING" != "no"; then

  dnl check ZTS support
  if test "$PHP_THREAD_SAFETY" == "yes"; then
    AC_MSG_ERROR([molten does not support ZTS])
  fi

  dnl check mmap functions
  AC_CHECK_FUNCS(mmap)
  AC_CHECK_FUNCS(munmap)

  dnl check for php json
  AC_MSG_CHECKING([check for php json])
  json_inc_path=""
  if test -f "$abs_srcdir/include/php/ext/json/php_json.h"; then
    json_inc_path="$abs_srcdir/include/php"
  elif test -f "$abs_srcdir/ext/json/php_json.h"; then
    json_inc_path="$abs_srcdir"
  elif test -f "$phpincludedir/ext/json/php_json.h"; then
    json_inc_path="$phpincludedir"
  else
    for i in php php4 php5 php6 php7; do
      if test -f "$prefix/include/$i/ext/json/php_json.h"; then
        json_inc_path="$prefix/include/$i"
      fi
    done
  fi

  if test "$json_inc_path" = ""; then
    AC_MSG_ERROR([cano not find php_json.h])
  else
    AC_MSG_RESULT([has found php json include file])
  fi
   
  dnl check for mysqli
  AC_MSG_CHECKING([check for mysqli])
  mysqli_inc_path=""
  if test -f "$abs_srcdir/include/php/ext/mysqli/php_mysqli_structs.h"; then
    mysqli_inc_path="$abs_srcdir/include/php"
  elif test -f "$abs_srcdir/ext/mysqli/php_mysqli_structs.h"; then
    mysqli_inc_path="$abs_srcdir"
  elif test -f "$phpincludedir/ext/mysqli/php_mysqli_structs.h"; then
    mysqli_inc_path="$phpincludedir"
  else
    for i in php php4 php5 php6 php7; do
      if test -f "$prefix/include/$i/ext/mysqli/php_mysqli_structs.h"; then
        mysqli_inc_path="$prefix/include/$i"
      fi
    done
  fi

  if test "mysqli_inc_path" = ""; then
      AC_MSG_RESULT([mysqli not found, mysqli support will not complete])
  else
      AC_MSG_RESULT([hash found mysqli include file])
      AC_DEFINE(HAS_MYSQLI, 1, [we have mysqli])
  fi

  dnl check for mysqlnd
  AC_MSG_CHECKING([check for mysqlnd])
  mysqlnd_inc_path=""
  if test -f "$abs_srcdir/include/php/ext/mysqlnd/mysqlnd_structs.h"; then
    mysqlnd_inc_path="$abs_srcdir/include/php"
  elif test -f "$abs_srcdir/ext/mysqlnd/mysqlnd_structs.h""; then
    mysqlnd_inc_path="$abs_srcdir"
  elif test -f "$phpincludedir/ext/mysqlnd/mysqlnd_structs.h""; then
    mysqlnd_inc_path="$phpincludedir"
  else
    for i in php php4 php5 php6 php7; do
      if test -f "$prefix/include/$i/ext/mysqlnd/mysqlnd_structs.h"; then
        mysqlnd_inc_path="$prefix/include/$i"
      fi
    done
  fi

  if test "$mysqlnd_inc_path" = ""; then
      AC_MSG_RESULT([mysqlnd not found, mysqli support will not complete])
  else
      AC_MSG_RESULT([has found mysqlnd include file])
      AC_DEFINE(HAS_MYSQLND, 1, [we have mysqlnd to support mysqli])
  fi

  dnl check for pdo
  AC_MSG_CHECKING([check for pdo])
  pdo_inc_path=""
  if test -f "$abs_srcdir/include/php/ext/pdo/php_pdo_driver.h"; then
    pdo_inc_path="$abs_srcdir/include/php"
  elif test -f "$abs_srcdir/ext/pdo/php_pdo_driver.h"; then
    pdo_inc_path="$abs_srcdir"
  elif test -f "$phpincludedir/ext/pdo/php_pdo_driver.h"; then
    pdo_inc_path="$phpincludedir"
  else
    for i in php php4 php5 php6 php7; do
      if test -f "$prefix/include/$i/ext/pdo/php_pdo_driver.h"; then
        pdo_inc_path="$prefix/include/$i"
      fi
    done
  fi

  if test "$pdo_inc_path" = ""; then
      AC_MSG_RESULT([pdo not found, pdo support will not complete])
  else
      AC_MSG_RESULT([has found pdo include file])
      AC_DEFINE(HAS_PDO, 1, [we support pdo])
  fi

  dnl check for curl
  AC_MSG_CHECKING([for curl-config])
  CURL_CONFIG="curl-config"
  CURL_CONFIG_PATH=`$php_shtool path $CURL_CONFIG`

  dnl for curl-config
  if test -f "$CURL_CONFIG_PATH" && test -x "$CURL_CONFIG_PATH" && $CURL_CONFIG_PATH --version > /dev/null 2>&1; then
      AC_MSG_RESULT([$CURL_CONFIG_PATH])
      CURL_LIB_NAME=curl
      dnl CURL_INCLUDE=`$CURL_CONFIG_PATH --prefix`/include/curl
      PHP_CHECK_LIBRARY($CURL_LIB_NAME, curl_easy_init, [
        dnl add curl include dir,  the lib dir in general path
        dnl PHP_EVAL_INCLINE($CURL_INCLUDE)
        PHP_ADD_LIBRARY($CURL_LIB_NAME,1,CURL_SHARED_LIBADD)
        PHP_SUBST(CURL_SHARED_LIBADD)
        AC_DEFINE(HAS_CURL, 1, [we have curl to execute curl])
      ] ,[
        AC_MSG_ERROR([libcurl not found])
     ])
  fi

  dnl check librdkafka
  KAFKA_SEARCH_PATH="/usr/local /usr"
  KAFKA_SEARCH_FOR="/include/librdkafka/rdkafka.h"
  AC_MSG_CHECKING([for librdkafka/rdkafka.h])
  for i in $KAFKA_SEARCH_PATH; do
      if test -r $i/$KAFKA_SEARCH_FOR; then

          KAFKA_DIR=$i
          AC_MSG_RESULT(found in $i)
          PHP_ADD_INCLUDE($KAFKA_DIR/include)

          LIBNAME=rdkafka
          LIBSYMBOL=rd_kafka_new
          PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,[
            PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $KAFKA_DIR/$PHP_LIBDIR, KAFKA_SHARED_LIBADD)
            AC_DEFINE(HAS_KAFKA, 1, [we have kafka to execute])
            PHP_SUBST(KAFKA_SHARED_LIBADD)
          ], [
            AC_MSG_ERROR([wrong rdkafka lib not found])
          ], [
            -L$KAFKA_DIR/$PHP_LIBDIR -lm
          ])
      fi
  done

  PHP_MOLTEN_SOURCE_FILES="\
    molten.c \
    molten_log.c \
    molten_intercept.c \
    molten_ctrl.c \
    common/molten_shm.c \
    common/molten_lock.c \
    molten_util.c \
    molten_span.c \
    molten_status.c \
    molten_report.c \
    molten_chain.c"

  dnl $ext_srcdir available after PHP_NEW_EXTENSION
  PHP_NEW_EXTENSION(molten, $PHP_MOLTEN_SOURCE_FILES, $ext_shared)

  dnl add common include path
  PHP_ADD_INCLUDE($ext_srcdir)
  PHP_ADD_INCLUDE($ext_srcdir/common)
  dnl PHP_ADD_INCLUDE($ext_srcdir/deps)

  dnl PHP_ADD_MAKEFILE_FRAGMENT

fi
