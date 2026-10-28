# UCM to JNI

## 一、说明
1. 将 UCM 编译成动态库形式 libucm.so + adm_ucm_apk.h
2. 将 c/c++ 的动态库封装成 jni 接口和库 libcom_abupdate_vus_service.so + jni.h/com_abupdate_vus_ucm_service.h
3. android 可以dlopen libcom_abupdate_vus_service.so 并使用 com_abupdate_vus_ucm_service.h 接口。
