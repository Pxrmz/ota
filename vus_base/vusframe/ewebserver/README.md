#分支说明：
 master: 主分支，公版代码管理分支，其他分支上开发完成后，需要由负责人统一合入；
 develop: 开发分支，后续的开发在此分支上进行，开发完成后，需要合入到master分支；
 develop-source: cievtweb源码分支；

#说明
    civetweb             webserver内核v1.13
    civetweb/openssl     openssl v1.11i
    sdk/include     
        lib              ewebsrv_static.a
        sample.c         demo sample

##打包
###linux/armlinux ewebsrv_static.a
   make.sh 
    make.sh 
    make version platform
    调用civetweb/cmake生成ewebsrv_static.a

###android  ewebsrv_static.a
     * 环境设置
     .bashrc
     export NDKROOT=/home/xjgu/Android/Sdk/ndk/23.0.7123448
     export PATH=$NDKROOT:$PATH

     * 编译
     cd jni
     ndk-build
     打包生成ewebsrv_static.a
     ./makeandroidstatic.sh obj/local

##编译ewebserver
cd civetweb/out
cmake .. 
make

##单独编译openssl

###linux/armlinux
cd openssl
./config --prefix=./ no-shared no-zlib no-asm  no-sse2 no-comp no-dso -DOPENSSL_THREADS -pthread -D_REENTRANT -D_THREAD_SAFE -D_THREADSAFE   
   当前目录，静态编译的形式，采用线程安全

指定--cross-compile-prefix=
$ sed -i "s/-m64//g" Makefile

或修改生成的Makefile文件，
   CC= arm-linux-gcc         
   AR= arm-linux-ar     
   RANLIB= arm-linux-ranlib

make
生成libssl.a libcrypto.a        

### android平台
./build-openssl-androidlib.sh
生成output/*/libssl.a libcrypto.a


---
##测试
GET:
curl -k --insecure "https://127.0.0.1:8080/demo" --output a
POST:
curl -k --insecure https://127.0.0.1:8080/a"  -F "file=@a" -v


##VUS项目编译
./make.sh v1.0 arm
#d第一个参数是版本名称，第二个参数是编译的平台类型

./make.sh v1.0 linux ##X86平台
./make.sh v1.0 arm ##arm平台
./make.sh v1.0 android ##android平台