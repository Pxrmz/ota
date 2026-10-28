#include <openssl/evp.h>


// 加密字符串   

void adm_encrpty()
{
	D8U* data = "12345";
	D8U* encrypt = adm_iss_rsaPubEncrypt(PUBKEY_RSA2048, data, RSA_PKCS1_PADDING);
	
	log_i("encryptData data:%s", encrypt);

	D8U* decryptData = adm_iss_rsaPubDecrypt(PUBKEY_RSA2048, encrypt, RSA_PKCS1_PADDING);

	log_i("decryptData data:%s", decryptData);
}

void evelop(EVP_PKEY* evp_pubKey/*公钥对象*/,BYTE** out/*加密结果*/,unsigned int &cOut/*加密结果大小*/,const BYTE* in/*明文*/,unsigned int cIn/*明文大小*/){  
    OpenSSL_add_all_algorithms();  
    bool hasErr = true;  
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(evp_pubKey,NULL);  
    if(!ctx)  
        goto err;  
    if(EVP_PKEY_encrypt_init(ctx)<=0)  
        goto err;  
    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)//设置补齐方式  
        goto err;  
  
    if(EVP_PKEY_encrypt(ctx,NULL,&cOut,in,cIn)<=0)//得到加密的长度,一般都是跟密钥一样长,1024位的RSA密钥就是128字节  
        goto err;  
    *out =(BYTE*) OPENSSL_malloc(cOut);  
    if(!out)  
        goto err;  
  
    if(EVP_PKEY_encrypt(ctx,*out,&cOut,in,cIn)<=0)//得到加密结果  
        goto err;  
    cout << "加密成功!" << endl;  
    hasErr = false;  
err:  
    EVP_PKEY_CTX_free(ctx);  
    if(!hasErr)  
        return;  
    return;  
}  	

void develop(EVP_PKEY* priKey,BYTE** out,unsigned int &cOut, BYTE* data,unsigned int cData){  
    OpenSSL_add_all_algorithms();  
    bool hasErr = true;  
    EVP_PKEY_CTX* ctx = EVP_PKEY_CTX_new(priKey,NULL);  
    if(!ctx)  
        goto err;  
    if (EVP_PKEY_decrypt_init(ctx) <= 0)  
        goto err;  
    if(EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_PADDING) <= 0)//与加密同样的PKCS1补齐  
        goto err;  
    if (EVP_PKEY_decrypt(ctx, NULL, &cOut, data, cData) <= 0)//解密结果大小  
        goto err;  
    *out =(BYTE*) OPENSSL_malloc(cOut);//分配内存  
    if (!out)  
        goto err;  
    if (EVP_PKEY_decrypt(ctx, *out, &cOut, data, cData) <= 0)//得到解密结果  
        goto err;  
    hasErr = false;  
err://输出一些错误信息,偶尔错误信息得不到,只得到一些文件名和所在行,就跑到源代码去找...麻烦死了,如果有好的方案求回复~  
    EVP_PKEY_CTX_free(ctx);  
    if(!hasErr)  
        return;  
    cout << "err in develop" << endl;  
  
    const      char*file,*data1,*efunc,*elib,*ereason,*p;  
  
    int                         line,flags;  
    unsigned long  errn;  
  
  
    errn=ERR_peek_error_line_data(&file,&line,&data1,&flags);  
  
    printf("ERR_peek_error_line_data err : %ld,file :%s,line :%d,data :%s\n",errn,file,line,data1);  
  
  
}


int main()
{
	adm_encrpty();
}
