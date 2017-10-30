VERSION=$(cat Makefile | grep EXTRAV | grep 0556 | awk -F"-" '{print $2"-"$3}')
export CROSS_COMPILE=/opt/0556_evb_pop/sysroots/x86_64-pokysdk-linux/usr/bin/arm-poky-linux-gnueabi/arm-poky-linux-gnueabi-
export KCFLAGS=--sysroot=/opt/0556_evb_pop/sysroots/cortexa9hf-vfp-neon-poky-linux-gnueabi
export KCPPFLAGS=
export KAFLAGS=
make distclean
make egf_evb_mx6_spl_config
make -j8
cp SPL binaries/SPL-$VERSION
cp u-boot.img binaries/u-boot.img-$VERSION
