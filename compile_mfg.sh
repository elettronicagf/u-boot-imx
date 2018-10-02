VERSION=$(cat Makefile | grep EXTRAV | grep 0556 | awk -F"-" '{print $2"-"$3}')
export CROSS_COMPILE=/opt/0556_evb_pop/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
export KCFLAGS=--sysroot=/opt/0556_evb_pop/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi
export KCPPFLAGS=
export KAFLAGS=
make distclean
make egf_evb_mx6_mfg_wid0500aa0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0500aa0101-$VERSION
make egf_evb_mx6_mfg_wid0500ab0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0500ab0101-$VERSION
make egf_evb_mx6_mfg_wid0500ac0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0500ac0101-$VERSION
make egf_evb_mx6_mfg_wid0500ad0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0500ad0101-$VERSION
make egf_evb_mx6_mfg_wid0500ae0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0500ae0101-$VERSION
