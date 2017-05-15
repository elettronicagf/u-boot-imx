VERSION=$(cat Makefile | grep EXTRAV | grep 0547 | awk -F"-" '{print $2"-"$3}')
make distclean
export KCFLAGS=--sysroot=/opt/0533_panel_pc/sdk/toolchain/1.0/sysroots/cortexa9hf-vfp-neon-egf-linux-gnueabi
export KCPPFLAGS=
export KAFLAGS=
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx7_mfg_wid0547_aa0101_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
cp u-boot.imx binaries/u-boot.imx.wid0547aa0101-$VERSION
