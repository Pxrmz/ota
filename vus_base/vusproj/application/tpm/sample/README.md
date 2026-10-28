## admtpmsample 使用说明

### ewebserver 及 传输文件清单

> * ewebserver:  ewebsrvdemo + config.txt + server.pem
> * 传输文件： httpUpdatePackage.iso



### ewebserver 配置及升级包存储位置配置

> 1. **将 ewebsrvdemo、config.txt、server.pem 拷贝到设备上 (与 admtpmsample在同一设备上)**；
>
> 2. 修改config.txt 文件，(/tmp/package/   为待传输文件存储路径);
>
>    ~~~
>    document_root /tmp/package/
>    document_uploaddir /tmp/package/
>    listening_ports 8082
>    
>    **** 下面保存原样 ****
>    ~~~
>
> 3. 在设备上新建目录 /tmp/package/download/， 并将待传输文件(httpUpdatePackage.iso)拷贝到新建的文件夹下(/tmp/package/download/httpUpdatePackage.iso);
>
> 4. 启动 ewebsrvdemo 服务
>
>    ~~~
>    chmod 777 ewebsrvdemo
>    ./ewebsrvdemo &
>    ~~~



### 测试 admtpmsample

>1. 执行 admtpmsample；
>2. 执行 sysi4trans 服务 admSysi4transhostSample;
>3. 传输完成后文件存储在 sysi4trans 侧 ADM_COM_PACKAGE_DIR 下，文件名为 httpUpdatePackage.iso
>
>