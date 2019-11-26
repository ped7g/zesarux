#!/bin/bash


INSTALLPREFIX=`cat compileoptions.h |grep INSTALL_PREFIX|cut -d '"' -f2`


cat > install.sh << _EOF
#!/bin/bash

echo "Installing ZEsarUX under $INSTALLPREFIX ..."

mkdir -p $INSTALLPREFIX
mkdir -p $INSTALLPREFIX/bin
mkdir -p $INSTALLPREFIX/share/zesarux/
mkdir -p $INSTALLPREFIX/share/zesarux/licenses/

cp zesarux $INSTALLPREFIX/bin/
cp *.rom zxuno.flash tbblue.mmc $INSTALLPREFIX/share/zesarux/

cp mantransfev3.bin $INSTALLPREFIX/share/zesarux/
cp editionnamegame.tap editionnamegame.tap.config $INSTALLPREFIX/share/zesarux/

cp -r speech_filters $INSTALLPREFIX/share/zesarux/
cp -r my_soft $INSTALLPREFIX/share/zesarux/

cp ACKNOWLEDGEMENTS Changelog HISTORY README FEATURES LICENSE LICENSES_info INSTALL INSTALLWINDOWS ALTERNATEROMS INCLUDEDTAPES DONATE FAQ $INSTALLPREFIX/share/zesarux/
cp licenses/* $INSTALLPREFIX/share/zesarux/licenses/
find $INSTALLPREFIX/share/zesarux/ -type f -print0| xargs -0 chmod 444

#chmod +x $INSTALLPREFIX/share/zesarux/macos_say_filter.sh
chmod +x $INSTALLPREFIX/share/zesarux/speech_filters/*

echo "Install done"

_EOF


chmod 755 install.sh

