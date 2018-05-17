VERSION=$(cat Makefile | grep EXTRAV | grep 0533 | awk -F"-" '{print $2"-"$3}')
make distclean
export CROSS_COMPILE=/opt/fsl-imx-x11/4.1.15-1.2.0/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
export KCFLAGS=--sysroot=/opt/fsl-imx-x11/4.1.15-1.2.0/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi
export KCPPFLAGS=
export KAFLAGS=
make egf_evb_mx6_mfg_wid0510aa0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510aa0101-$VERSION
make egf_evb_mx6_mfg_wid0510ab0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ab0101-$VERSION
make egf_evb_mx6_mfg_wid0510ac0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ac0101-$VERSION
make egf_evb_mx6_mfg_wid0510ac0102_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ac0102-$VERSION
make egf_evb_mx6_mfg_wid0510ae0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ae0101-$VERSION
make egf_evb_mx6_mfg_wid0510ae0102_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ae0102-$VERSION
make egf_evb_mx6_mfg_wid0510ad0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ad0101-$VERSION
make egf_evb_mx6_mfg_wid0510af0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510af0101-$VERSION
make egf_evb_mx6_mfg_wid0510af0102_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510af0102-$VERSION
make egf_evb_mx6_mfg_wid0510ag0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ag0101-$VERSION
make egf_evb_mx6_mfg_wid0510ag0102_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ag0102-$VERSION
make egf_evb_mx6_mfg_wid0510aj0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510aj0101-$VERSION
make egf_evb_mx6_mfg_wid0510aj0102_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510aj0102-$VERSION
make egf_evb_mx6_mfg_wid0510ak0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ak0101-$VERSION
make egf_evb_mx6_mfg_wid0510ak0102_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ak0102-$VERSION
make egf_evb_mx6_mfg_wid0510an0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510an0101-$VERSION
