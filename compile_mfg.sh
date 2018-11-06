VERSION=$(cat Makefile | grep EXTRAV | grep 0574 | awk -F"-" '{print $2"-"$3}')
make distclean
export CROSS_COMPILE=/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
export KCFLAGS=--sysroot=/opt/fsl-imx-x11/4.1.15-2.1.0/sysroots/cortexa7hf-neon-poky-linux-gnueabi/
export KCPPFLAGS=
export KAFLAGS=
make egf_evb_mx7_mfg_wid0575_aa0101_config
make -j8
cp u-boot.imx binaries/u-boot.imx.wid0575aa0101-$VERSION
make egf_evb_mx7_mfg_wid0575_ab0101_config
make -j8
cp u-boot.imx binaries/u-boot.imx.wid0575ab0101-$VERSION
make egf_evb_mx7_mfg_wid0575_aa0102_config
make -j8
cp u-boot.imx binaries/u-boot.imx.wid0575aa0102-$VERSION
