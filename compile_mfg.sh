VERSION=$(cat Makefile | grep EXTRAV | grep 0569 | awk -F"-" '{print $2"-"$3}')

export CROSS_COMPILE=/opt/fsl-imx-x11/4.14-sumo/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
export KCFLAGS=--sysroot=/opt/fsl-imx-x11/4.14-sumo/sysroots/cortexa7hf-neon-poky-linux-gnueabi/
export KCPPFLAGS=
export KAFLAGS=

make distclean
make egf_sbc_mx6ull_mfg_wid0569_aa0101_config
make -j8
cp u-boot-dtb.imx binaries/u-boot.imx.wid0569aa0101-$VERSION

