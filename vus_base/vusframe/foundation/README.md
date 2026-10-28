# foundation的编译 -- 要求cmake的版本不低于3.17

## X86编译:
### ./make.sh x86 v1.1    ##./make.sh 选择的平台 发布的包名

## ARM编译: 类似的有9x07 qnx7
## 编译前source env
### ./make.sh im8x v1.2 ##./make.sh 选择的平台 发布的包名

## NDK编译: 
## 编译前source env
### ./make.sh android v1.3 ##./make.sh 选择的平台 发布的包名

## 关于foundation链接openssl的编译选项在cmake目录下文件设置
### USE_SYSTEM_SSL=ON使用系统SSL动态库库，USE_SYSTEM_SSL=OFF使用工程生成的库
### USE_SYSTEM_SSL=OFF时，USE_DYNAMIC_SSL_LIB=ON使用工程生成的动态库，USE_DYNAMIC_SSL_LIB=OFF使用工程生成的静态库
### 缺省使用工程生成的静态库
### 若使用系统的动态库请在cmake/目录下选择文件指定OPENSSL的根目录<===>OPENSSL_ROOT_DIR，见toolchain-imx8-a35.cmake

## 部分编译选项设置见cmake目录下的编译文件

