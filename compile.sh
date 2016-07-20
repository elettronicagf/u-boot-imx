#VERSION=$(cat Makefile | grep EXTRAV | grep 0533 | awk -F"-" '{print $2"-"$3}')
make distclean
make CROSS_COMPILE=arm-egf-linux-gnueabi- egf_evb_mx6_spl_config
make CROSS_COMPILE=arm-egf-linux-gnueabi- -j8
#cp SPL binaries/SPL-$VERSION
#cp u-boot.img binaries/u-boot.img-$VERSION
