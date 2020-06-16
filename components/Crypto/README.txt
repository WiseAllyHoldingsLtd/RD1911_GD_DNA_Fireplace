
ATMEL/MicroChip CryptoAuthLib component
============================================

cryptoauthlib is written in C and with syntax uncompatible with C++.
In order to compile it in our C++ project we need to modify atca_iface.h
slightly. The change can be seen in overrides/cpp_compat.patch.

With clever include order we make sure our modified header file is found
before the original and thus letting us use it in C++.

Due to a limit in the IDF build system we cannot include single SRC files
and to compile only atca_hal.c in lib/hal we copy it to a separate folder and
build everything in that folder (overrides).

If the version of cryptoauthlib is ever changed it is very important
that these two files are updated aswell.

Ugrading
==========
When the cryptoauthlib version has been changed, please run the update.sh
script in overrides/ to make sure our modified files are upgraded as well.
