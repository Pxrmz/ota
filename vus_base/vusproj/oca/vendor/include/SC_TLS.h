/*
 * Copyright (C) kong
 */

#ifndef SCM_SC_TLS_H
#define SCM_SC_TLS_H

#include "SC_PKI.h"
#include <openssl/ssl.h>
#include <curl/curl.h>

#ifdef SCM_MQTT
#include <mosquitto/mosquitto.h>
#endif

#ifdef _WIN32
#define SC_SSLEXPORT __declspec(dllexport)
#else
#define SC_SSLEXPORT __attribute__ ((visibility ("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

SC_SSLEXPORT SC_RESULT
IW_GetSSLCtx(SC_HANDLE hHandle, const char *userID, const char *pin, SSL_CTX **ctx);

SC_SSLEXPORT CURLcode
curl_easy_setopt_cert_and_privatekey(SC_HANDLE hHandle, CURL *curl, const char *userID, const char *userPIN);

SC_SSLEXPORT CURLcode
curl_easy_setopt_cert_and_privatekey_gm(SC_HANDLE hHandle, CURL *curl, const char *userID, const char *userPIN);

#ifdef SCM_MQTT
SC_SSLEXPORT SC_RESULT
mosquitto_opts_set_cert_and_privatekey(SC_HANDLE hHandle, struct mosquitto *mosq, const char *userID, const char *userPIN);
#endif

#ifdef __cplusplus
}
#endif

#endif //SCM_SC_TLS_H
