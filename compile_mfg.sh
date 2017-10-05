VERSION=$(cat Makefile | grep EXTRAV | grep 0533 | awk -F"-" '{print $2"-"$3"-"$4"-"$5 }')
make distclean
export CROSS_COMPILE=/opt/0533_panel_pc/sdk/toolchain/1.0/sysroots/x86_64-egf-linux/usr/bin/arm-egf-linux-gnueabi/arm-egf-linux-gnueabi-
export KCFLAGS=--sysroot=/opt/0533_panel_pc/sdk/toolchain/1.0/sysroots/cortexa9hf-vfp-neon-egf-linux-gnueabi
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
make egf_evb_mx6_mfg_wid0510ae0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ae0101-$VERSION
make egf_evb_mx6_mfg_wid0510ad0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ad0101-$VERSION
make egf_evb_mx6_mfg_wid0510af0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510af0101-$VERSION
make egf_evb_mx6_mfg_wid0510ag0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ag0101-$VERSION
make egf_evb_mx6_mfg_wid0510aj0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510aj0101-$VERSION
make egf_evb_mx6_mfg_wid0510ak0101_config
make -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ak0101-$VERSION
