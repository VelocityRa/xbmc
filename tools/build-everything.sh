#!/bin/bash

# Script for testing build status of all depends targets for Switch

# List of deps:
# Removing for now and only leave the required ones
#    "pcre" "expat" "sqlite3" "libgpg-error"
#    "libgcrypt" "bzip2" "libfstrcmp" "liblzo2" "libzip" "freetype2" "fontconfig"
#    "openssl" "gmp" "nettle" "gnutls" "curl"
#    "libjpeg-turbo" "libpng" "fribidi" "libass"
#    "libxml2" "rapidjson" "libmicrohttpd" "mariadb" "libffi"
#    "libshairplay" "libfmt"
#    "libplist" "libbluray" "tinyxml" "taglib" "libnfs"
#    "pythonmodule-pil" "pythonmodule-pycryptodome" "pythonmodule-setuptools"
#    "libxslt" "ffmpeg" "platform" "crossguid"
#    "libdvdread" "libdvdnav" "libdvdcss" "p8-platform" "flatbuffers" "libiconv"
#    "python27")

# List of required deps:
#    "pcre" "flatbuffers" "libiconv" "crossguid" "curl" "libass" "libfmt" "fribidi"
#    "libfstrcmp" "libdvdread" "libdvdnav" "libdvdcss" "liblzo2" "openssl"
#    "rapidjson" "sqlite3" "taglib" "tinyxml" "libcdio" "libcdio-gplv3"

# List of deps that are required and NOT available as a portlib
declare -a targets=(
    "pcre" "flatbuffers" "libiconv" "crossguid" "libfmt"
    "libfstrcmp" "libdvdread" "libdvdnav" "libdvdcss" "liblzo2" "openssl"
    "rapidjson" "sqlite3" "taglib" "tinyxml" "libcdio" "libcdio-gplv3")

# required targets for which we can use a portlib:
# - ffmpeg
# - libass
# - curl
# - libfribidi
# - libexpat
# - zlib (?)

rm switch-built.txt
rm switch-not-built.txt

touch switch-built.txt
touch switch-not-built.txt

for t in "${targets[@]}"
do
    make -C "target/$t"
    if [ -f target/$t/.installed-aarch64-none-elf-release ]; then
        echo "$t" >> switch-built.txt
    else
        echo "$t" >> switch-not-built.txt
    fi
done
