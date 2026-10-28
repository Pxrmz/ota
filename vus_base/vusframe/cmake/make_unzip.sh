#!/bin/bash -e
### 脚本描述：
### 代码下载后，在编译之前，需要执行此脚本，解压所有目录下的库文件到对应目录下

TOPDIR=$(pwd)

# foundation库的压缩包生成后，需要解压到foundation_lib目录下
function foundation_unzip() {
    tar -zxvf $TOPDIR/../foundation_lib/foundation*.tar.gz -C $TOPDIR/../foundation_lib/
}

# ewebserver库的压缩包生成后，需要解压到ewebserver_lib目录下
function ewebserver_unzip() {
    tar -zxvf $TOPDIR/../ewebserver_lib/ewebserver*.tar.gz -C $TOPDIR/../ewebserver_lib/
}

# tools库的压缩包生成后，需要解压到tools_lib目录下
function tools_unzip() {
    tar -zxvf $TOPDIR/../tools_lib/tools*.tar.gz -C $TOPDIR/../tools_lib/
}

# engine库的压缩包生成后，需要解压到engine_lib目录下
function engine_unzip() {
    tar -zxvf $TOPDIR/../engine_lib/engine*.tar.gz -C $TOPDIR/../engine_lib/
}

# arpc库的压缩包生成后，需要解压到arpc_lib目录下
function arpc_unzip() {
    tar -zxvf $TOPDIR/../arpc_lib/arpc*.tar.gz -C $TOPDIR/../arpc_lib/
}

# vrpc库的压缩包生成后，需要解压到vrpc_lib目录下
function vrpc_unzip() {
    tar -zxvf $TOPDIR/../vrpc_lib/vrpc*.tar.gz -C $TOPDIR/../vrpc_lib/
}

foundation_unzip
ewebserver_unzip
tools_unzip
engine_unzip
arpc_unzip
# vrpc_unzip