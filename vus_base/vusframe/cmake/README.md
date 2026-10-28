# 编译步骤：
## 解压库文件
    $ ./make_unzip.sh
## 执行编译
### 单独编译编译各源码
    $ ./make_engine.sh <PLATFORM> <VERSION>
    $ ./make_foundation.sh <PLATFORM> <VERSION>
    $ ./make_tools.sh <PLATFORM> <VERSION>
    $ ./make_arpc.sh <PLATFORM> <VERSION>
    $ ./make_vrpc.sh <PLATFORM> <VERSION>
### 统一编译vusframe仓库
    $ ./make_vusframe.sh <PLATFORM> <VERSION>