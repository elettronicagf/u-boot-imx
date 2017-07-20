VERSION=$(cat Makefile | grep EXTRAV | grep 0533 | awk -F"-" '{print $2"-"$3}')
make distclean
export CROSS_COMPILE=/opt/0533_panel_pc/sdk/toolchain/1.0/sysroots/x86_64-egf-linux/usr/bin/arm-egf-linux-gnueabi/arm-egf-linux-gnueabi-
export KCFLAGS=--sysroot=/opt/0533_panel_pc/sdk/toolchain/1.0/sysroots/cortexa9hf-vfp-neon-egf-linux-gnueabi
export KCPPFLAGS=
export KAFLAGS=
make egf_evb_mx6_spl_config
make -j8
cp SPL binaries/SPL-$VERSION
cp u-boot.img binaries/u-boot.img-$VERSION
