# This dockerfile uses php image
# 使用php:7官方的基础镜像
FROM php:7.1-fpm-alpine

# 更换国内源
RUN  { \
   echo 'http://mirrors.aliyun.com/alpine/v3.4/community'; \
   echo 'http://mirrors.aliyun.com/alpine/v3.4/main'; \
} | tee /etc/apk/repositories

# 镜像的环境变量
ENV MOLTEN_VERSION="v0.1.1"

# 安装依赖库
RUN apk update &&  apk add --no-cache \
       git \
       linux-headers \
       libcurl \
       curl-dev

## 安装php扩展
RUN docker-php-source extract \
      && git clone -b $MOLTEN_VERSION https://gitee.com/tools/Molten.git /usr/src/php/ext/molten \
      \
      && docker-php-ext-configure molten \
        --enable-molten \
        --enable-zipkin-header=yes \
      \
      && docker-php-ext-install molten \
      && docker-php-source delete

WORKDIR /var/www
