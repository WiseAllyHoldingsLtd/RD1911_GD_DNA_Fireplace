#pragma once


#ifndef UNITTESTS
#include "mbedtls/base64.h"
#else
#include <cstdint>
int mbedtls_base64_encode(unsigned char*, size_t, size_t*, const unsigned char*, size_t);
int mbedtls_base64_decode(unsigned char*, size_t, size_t*, const unsigned char*, size_t);
#endif
