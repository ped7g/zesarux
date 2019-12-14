#!/bin/bash


INSTALLPREFIX=`cat compileoptions.h |grep INSTALL_PREFIX|cut -d '"' -f2`


cat > install.sh << _EOF
#!/bin/bash

echo "Installing ZESERUse under $INSTALLPREFIX ..."

# make already installed files writeable to overwrite them
if [[ -f $INSTALLPREFIX/share/zeseruse/README ]]; then
	echo "Seems there is already ZESERUse installed, this will be simply overwritten"
	# make all files already installed write-able, so the installer can overwrite any of them
	find $INSTALLPREFIX/share/zeseruse/ -type f -print0| xargs -0 chmod +w
fi

mkdir -p $INSTALLPREFIX
mkdir -p $INSTALLPREFIX/bin
mkdir -p $INSTALLPREFIX/share/zeseruse/
mkdir -p $INSTALLPREFIX/share/zeseruse/licenses/

cp zeseruse $INSTALLPREFIX/bin/
cp *.rom zxuno.flash tbblue.mmc $INSTALLPREFIX/share/zeseruse/

cp mantransfev3.bin $INSTALLPREFIX/share/zeseruse/
cp editionnamegame.tap editionnamegame.tap.config $INSTALLPREFIX/share/zeseruse/

cp -r speech_filters $INSTALLPREFIX/share/zeseruse/
cp -r my_soft $INSTALLPREFIX/share/zeseruse/

cp ACKNOWLEDGEMENTS Changelog HISTORY README FEATURES LICENSE LICENSES_info INSTALL INSTALLWINDOWS ALTERNATEROMS INCLUDEDTAPES DONATE FAQ $INSTALLPREFIX/share/zeseruse/
cp licenses/* $INSTALLPREFIX/share/zeseruse/licenses/
find $INSTALLPREFIX/share/zeseruse/ -type f -print0| xargs -0 chmod 444

#chmod +x $INSTALLPREFIX/share/zeseruse/macos_say_filter.sh
chmod +x $INSTALLPREFIX/share/zeseruse/speech_filters/*

echo "Install done"

_EOF


chmod 755 install.sh

