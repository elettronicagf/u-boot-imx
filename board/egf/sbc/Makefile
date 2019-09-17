obj-y  := egf_sbc_mx6.o gf_mux.o gf_eeprom.o gf_eeprom_port.o

extra-$(CONFIG_USE_PLUGIN) :=  plugin.bin
$(obj)/plugin.bin: $(obj)/plugin.o
	$(OBJCOPY) -O binary --gap-fill 0xff $< $@
