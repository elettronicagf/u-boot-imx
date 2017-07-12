VERSION=$(cat Makefile | grep EXTRAV | grep 0574 | awk -F"-" '{print $2"-"$3}')
make distclean
export CROSS_COMPILE=/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
export KCFLAGS=--sysroot=/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi/
export KCPPFLAGS=
export KAFLAGS=
make egf_evb_mx7_spl_config
make -j8
cp SPL binaries/SPL-$VERSION
cp u-boot.img binaries/u-boot.img-$VERSION
