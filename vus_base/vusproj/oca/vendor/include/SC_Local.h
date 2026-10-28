/*
 * Copyright (C) kong
 */

#ifndef SCM_SC_LOCAL_H
#define SCM_SC_LOCAL_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef _WIN32
#define PACKED_ST(st) __attribute__((packed, aligned(1))) st
#else
//#pragma pack(push, security, 1)
#define PACKED_ST(st) st
#endif

#endif //SCM_SC_LOCAL_H
