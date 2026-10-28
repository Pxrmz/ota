#ifndef __ASSERT_PARAM_H
#define __ASSERT_PARAM_H

#include "__assert.h"

#define ASSERT_PARAM(exper) \
    __ASSERT(exper, "%s", "invalid parameter");

#endif
