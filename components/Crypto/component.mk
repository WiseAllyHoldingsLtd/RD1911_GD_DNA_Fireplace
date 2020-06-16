#
# component makefile.
#
# This Makefile can be left empty. By default, it will take the sources in the
# src/ directory, compile them and link them into lib(subdirectory_name).a
# in the build directory. This behaviour is entirely configurable,
# please read the ESP-IDF documents if you need to do this.
#

COMPONENT_SRCDIRS := ./ cryptoauthlib/lib cryptoauthlib/lib/basic cryptoauthlib/lib/host cryptoauthlib/lib/tls cryptoauthlib/lib/atcacert cryptoauthlib/lib/crypto cryptoauthlib/lib/crypto/hashes overrides
COMPONENT_ADD_INCLUDEDIRS := ./ overrides cryptoauthlib/lib cryptoauthlib/lib/hal include

# Cherrypick needed HAL files (should work, but somehow doesnt.)
# COMPONENT_OBJINCLUDE := cryptoauthlib/lib/hal/atca_hal.o

# Needed to enable I2C headers
CPPFLAGS += -DATCA_HAL_I2C -DATCAPRINTF
