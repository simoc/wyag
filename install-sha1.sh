#!/bin/sh
# Download and unpack C++ source code for computing SHA1 hashes.
set -ex
wget -O sha1.zip https://github.com/vog/sha1/archive/master.zip
unzip sha1.zip
mv sha1-master sha1
