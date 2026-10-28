set(CMAKE_SYSTEM_NAME Linux)

set(CUSTOM_CMAKE_SYSTEM_NAME Linux)

set(TOOLCHAIN_IMX8_A35_NAME 1)

option(USE_EVG_CONTI_IMX8_A35_COMPLIER  "Use EVG Conti TBOX IMX8 A35 complier"  ON)

set(CUSTOM_CMAKE_XML_NAME MXML)

#是否使用系统的SSL库，ON使用系统的；OFF使用工程生成的
set(USE_SYSTEM_SSL ON)

#使用系统SSL 指明ssl根路径 
#set(USER_COMPILER_DIR /mnt/develop/toolchain/EVG)
#set(OPENSSL_ROOT_DIR ${USER_COMPILER_DIR}/customer_EVG_PT0_v1/release/fs/devel/usr)
set(OPENSSL_ROOT_DIR $ENV{CAS_WORKDIR}/release/fs/devel/usr)

set(USE_DYNAMIC_SSL_LIB OFF)



