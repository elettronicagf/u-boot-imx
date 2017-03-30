VERSION=$(cat Makefile | grep EXTRAV | grep 0533 | awk -F"-" '{print $2"-"$3}')
make distclean
export KCFLAGS=--sysroot=/opt/0533_panel_pc/sdk/toolchain/1.0/sysroots/cortexa9hf-vfp-neon-egf-linux-gnueabi
export KCPPFLAGS=
export KAFLAGS=
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_mfg_wid0510aa0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
mv u-boot.imx binaries/u-boot.imx.wid0510aa0101-$VERSION
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_mfg_wid0510ab0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ab0101-$VERSION
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_mfg_wid0510ac0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ac0101-$VERSION
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_mfg_wid0510ae0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ae0101-$VERSION
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_mfg_wid0510ad0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
mv u-boot.imx binaries/u-boot.imx.wid0510ad0101-$VERSION
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_mfg_wid0510af0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
mv u-boot.imx binaries/u-boot.imx.wid0510af0101-$VERSION

