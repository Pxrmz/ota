一、 基本linux平台的doxygen环境搭建
安装命令如下：
sudo apt-get install doxygen
sudo apt-get install graphviz
二、搭建生成中文文档的环境：
安装CTeX、CJKutf8 package环境，命令如下：
sudo apt-get install texlive-xetex latex-cjk-xcjk texlive-latex-recommended

三、快速生成中/英文 pdf 文档：
1. 英文手册配置模板(默认为英文版本，要求头文件不能出现中文字符): doxygen, doxyconfig, 
中文手册配置模板： doxygen_cn, doxyconfig_cn
在模板目录的include中均有中文版和英文版头文件样例，供参考。
2. 模板目录解析：
include：存放自己工程里需要生成pdf手册的头文件。
doxyconfig：配置文件。
doxygen：运行脚本。
bash_helper.sh：固定脚本（无需修改）
DoxygenLayout.xml：固定布局文件（无需修改）
3. 运行脚本，自动生成pdf文档：
进入模板目录，执行./doxygen，会在./doc/latex目录下生成手册refman.pdf。
4.doxygen常见修改说明：
PROJECT_NAME="VUS SDK Reference Manual"  //手册名称
PROJECT_NUMBER="v0.1.0"  //手册版本号
OUTPUT_DIRECTORY=$(normalize_path ${THIS_DIR}/doc) //设置手册输出目录
四、手册更新：
如果生成pdf之后，头文件有修改，需要重新生成手册：
1. rm doc -r
2.再执行./doxygen，会在./doc/latex目录下生成手册refman.pdf。

