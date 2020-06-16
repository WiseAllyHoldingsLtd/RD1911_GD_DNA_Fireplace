#!/usr/bin/sh
echo "Copying files"
cp ../cryptoauthlib/lib/hal/atca_hal.c .
cp ../cryptoauthlib/lib/atca_iface.h .

echo "Patching files"
dos2unix atca_iface.h  # Ensure there is no issues with patch and line endings (ASSUMES gitconfig.autocrlf=true)
dos2unix cpp_compat.patch
patch < cpp_compat.patch
unix2dos cpp_compat.patch
unix2dos atca_iface.h

echo "Done"
