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
