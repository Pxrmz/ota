/*
 * Copyright (C) kong
 */

#ifndef SCM_SC_DEFINE_H
#define SCM_SC_DEFINE_H

#include "SC_Local.h"

typedef struct  Buffer_st{
    size_t   bufferLen;
    uint8_t *buffer;
}PACKED_ST(SC_BufferInformation);

typedef struct SC_SubjectInformation_st {
    char C[32 + 1];
    char ST[32 + 1];
    char L[32 + 1];
    char O[64 + 1];
    char OU[64 + 1];
    char OU1[64 + 1];
    char SN[64 + 1];
    char CN[64 + 1];
    char DC1[32 + 1];
    char DC2[32 + 1];
    char DC3[32 + 1];
} PACKED_ST(SC_SubjectInformation);

typedef struct SC_AttributeInformation_st {
    char challengePassword[32];     /* N ChallengePassword */
    char subjectAltName[64];        /* N Subject Alternative Name */
} PACKED_ST(SC_AttributeInformation);

typedef struct SC_CSRInformation_st {
    SC_SubjectInformation   subjectInfo;
    SC_AttributeInformation attributeInfo;
} PACKED_ST(SC_CSRInformation);

#endif //SCM_SC_DEFINE_H
