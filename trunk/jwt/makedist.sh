#!/bin/sh

version=$1
[ x$version = x ] && echo "Version missing!" && exit 1

path=$(pwd)
dest=/tmp/jwt

rm -rf $dest
mkdir $dest $dest/pali
cp pali/*.js $dest/pali
cp changelog.txt copying.txt readme.txt 
cp jwt.js dialog.js pali.htm style.css $dest
cd /tmp
zip -r $path/jwt-${version}.zip jwt
rm -rf $dest
exit 0
