// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.
// TODO copyright notice...?

#include "azure_c_shared_utility/tlsio_mbedtls.h"
#include "azure_c_shared_utility/platform.h"
#include "tlsio_pal.h"
#include "azure_c_shared_utility/xlogging.h"

/* Codes_SRS_PLATFORM_OPENSSL_COMPACT_30_004: [ The platform_init shall initialize the tlsio adapter. ] */
int platform_init(void)
{
    /* no sntp client */
    return 0;
}

/* Codes_SRS_PLATFORM_OPENSSL_COMPACT_30_008: [ The platform_get_default_tlsio shall return a set of tlsio functions provided by the OpenSSL micro tlsio implementation. ] */
const IO_INTERFACE_DESCRIPTION* platform_get_default_tlsio(void)
{
    return tlsio_mbedtls_get_interface_description();
}

STRING_HANDLE platform_get_platform_info(void)
{
    return STRING_construct("(esp32_mbed)");
}

/* Codes_SRS_PLATFORM_OPENSSL_COMPACT_30_007: [ The platform_deinit shall deinitialize the tlsio adapter. ] */
void platform_deinit(void)
{

}
