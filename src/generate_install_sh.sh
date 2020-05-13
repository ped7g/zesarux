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

COMMONFILES="ACKNOWLEDGEMENTS LICENSE LICENSES_info licenses Changelog TODO* README HISTORY FEATURES INSTALL INSTALLWINDOWS ALTERNATEROMS INCLUDEDTAPES DONATE FAQ *.odt mantransfev3.bin *.rom zxuno.flash tbblue.mmc speech_filters my_soft zesarux.mp3 zesarux.xcf editionnamegame.tap editionnamegame.tap.config bin_sprite_to_c.sh"

cp -a \$COMMONFILES $INSTALLPREFIX/share/zeseruse/
cp zeseruse $INSTALLPREFIX/bin/

# Default permissions for files: read only
find $INSTALLPREFIX/share/zeseruse/ -type f -print0| xargs -0 chmod 444

# Speech filters can be run
chmod +x $INSTALLPREFIX/share/zeseruse/speech_filters/*

echo "Install done"

_EOF


chmod 755 install.sh

