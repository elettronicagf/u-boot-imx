#include <common.h>
#include "gf_eeprom_port.h"
#include <i2c.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <asm/arch/iomux.h>
#include <asm/arch/mx6-pins.h>
#include <asm/arch/sys_proto.h>
#include <asm/imx-common/boot_mode.h>
#include <asm/imx-common/iomux-v3.h>
#include <asm/imx-common/mxc_i2c.h>
DECLARE_GLOBAL_DATA_PTR;

struct wid_translation legacy_wid_translation_table[] = {
};

struct wid_list_for_eeprom_programming wid_list[] = {
	{"JSM0510_B01.Beta1", REV_WID0510_AA0101},
	{"JSM0510_E01.Beta1", REV_WID0510_AB0101},
	{"JSM0510_F01.Beta1", REV_WID0510_AC0101},
	{"JSM0510_F01", REV_WID0510_AC0101},
	{"JSM0510_F02", REV_WID0510_AC0102},
	{"JSM0510_H01.Beta1", REV_WID0510_AD0101},
	{"JSM0510_G01.Beta1", REV_WID0510_AE0101},
	{"JSM0510_G01", REV_WID0510_AE0101},
	{"JSM0510_G02", REV_WID0510_AE0102},
	{"JSM0510_L01.Beta1", REV_WID0510_AF0101},
	{"JSM0510_PB1.Beta1", REV_WID0510_AF0101},
	{"JSM0510_PB1", REV_WID0510_AF0102},
	{"JSM0510_P01.Beta1", REV_WID0510_AG0101},
	{"JSM0510_L02", REV_WID0510_AG0101},
	{"JSM0510_P01", REV_WID0510_AG0102},
	{"JSM0510_L03", REV_WID0510_AG0102},
	{"JSM0510_T01", REV_WID0510_AG0102},
	{"JSM0510_PC1.Beta1", REV_WID0510_AJ0101},
	{"JSM0510_PC1", REV_WID0510_AJ0102},
	{"JSM0510_PD1.Beta1", REV_WID0510_AK0101},
	{"JSM0510_PD1", REV_WID0510_AK0102},
	{"JSM0510_PF1.Beta1", REV_WID0510_AK0102},
	{"JSM0510_S01", REV_WID0510_AN0101},

};

void *gf_memset(void *s, int c, size_t n)
{
	size_t i;
	char *ptr = s;
	for (i=0;i<n;i++,ptr++)
	{
		*ptr = c;
	}
	return s;
}

size_t gf_strlen(const char * s)
{
	const char *sc;

	for (sc = s; *sc != '\0'; ++sc);
	return sc - s;
}

int gf_strncmp(const char * cs, const char * ct, size_t count) {
	register signed char __res = 0;

	while (count) {
		if ((__res = *cs - *ct++) != 0 || !*cs++)
			break;
		count--;
	}

	return __res;
}


void gf_strcpy (char dest[], const char src[])
{
	int i = 0;
	while (1)
	{
		dest[i] = src[i];
		if (dest[i] == '\0') break;
		i++;

	}
}

unsigned long gfstr_toul (
    char*   nstr,
    char**  endptr,
    int base)
{
    char* s = nstr;
    unsigned long acc;
    unsigned char c;
    unsigned long cutoff;
    int neg = 0, any, cutlim;

    do
    {
        c = *s++;
    } while (ISSPACE(c));

    if (c == '-')
    {
        neg = 1;
        c = *s++;
    }
    else if (c == '+')
        c = *s++;

    if ((base == 0 || base == 16) &&
        c == '0' && (*s == 'x' || *s == 'X'))
    {
        c = s[1];
        s += 2;
        base = 16;
    }
    if (base == 0)
        base = c == '0' ? 8 : 10;

    cutoff = -1UL/ (unsigned long)base;
    cutlim = -1UL % (unsigned long)base;
    for (acc = 0, any = 0; ; c = *s++)
    {
        if (!ISASCII(c))
            break;
        if (ISDIGIT(c))
            c -= '0';
        else if (ISALPHA(c))
            c -= ISUPPER(c) ? 'A' - 10 : 'a' - 10;
        else
            break;

        if (c >= base)
            break;
        if (any < 0 || acc > cutoff || (acc == cutoff && c > cutlim))
            any = -1;
        else
        {
            any = 1;
            acc *= base;
            acc += c;
        }
    }

    if (any < 0)
    {
        acc = ~0U >> 1;
    }
    else if (neg)
        acc = -acc;
    if (endptr != 0)
        *((const char **)endptr) = any ? s - 1 : nstr;
    return (acc);

}

void gf_u8tox(u8 i, char *s)
{
    unsigned char n;

    s += 2;
    *s = '\0';

    for (n = 2; n != 0; --n) {
        *--s = "0123456789ABCDEF"[i & 0x0F];
        i >>= 4;
    }
}

char * gf_strcat (char *dest, const char *src)
{
	while(*dest) dest++;
	while ((*dest++ = *src++));
	return dest;
}

void gf_i2c_init(void)
{
	return;
}

int gf_i2c_set_bus_num(unsigned int bus)
{
	return i2c_set_bus_num(bus);
}

int gf_i2c_probe (u8 chip)
{
	return i2c_probe(chip);
}

int gf_serial_getc(void)
{
	return serial_getc();
}

void gf_serial_init(void)
{
	return;
}

void gf_som_eeprom_unlock(void)
{
	gpio_set_value(EEPROM_nWP_GPIO, 1);
}

void gf_som_eeprom_lock(void)
{
	gpio_set_value(EEPROM_nWP_GPIO, 0);
}

int gf_eeprom_read(u8 address,u16 start_address,u8 * buffer,int len)
{
	int i;
	for(i=0; i<len; i++){
		if(i2c_read(address, i+start_address, 2, &buffer[i], 1)){
			return 1;
		}
	}
	return 0;
}

int gf_eeprom_write(u8 address,u16 start_address,u8 * buffer,int len)
{
	int i;
	int ret;
	for(i=0; i<len; i++){
		ret = i2c_write(address, i+start_address, 2, &buffer[i],1);
		if(ret){
			return 1;
		}
		udelay(10000);
	}
	return 0;
}

int gf_read_programmer_file(const char * file_name,char * file_buffer,int buffer_length)
{
	int index = 0;
	char c;
	int choice = -1;

	printf("Eeprom configurations available:\n");
	for (index = 0; index < WID_PROGRAMMING_TABLE_LENGTH; index++) {
		printf("%d) %s - %s\n", index, wid_list[index].product_code, wid_list[index].sw_id_code);
	}
	printf("Choose one configuration... ");


	while (1) {
		c = gf_serial_getc();
		printf("%c\n",c);
		choice = c-'0';
		if (choice < 0 || choice >= WID_PROGRAMMING_TABLE_LENGTH) {
			printf("Invalid Choice\n");
			printf("Choose one configuration... ");
		} else {
			break;
		}
	}

	file_buffer[0] = 0;
	gf_strcat(file_buffer,wid_list[choice].sw_id_code);
	gf_strcat(file_buffer,GFCONFIG_SEPARATOR);
	gf_strcat(file_buffer,wid_list[choice].product_code);
	gf_strcat(file_buffer,GFCONFIG_SEPARATOR);
	gf_debug(0,"%s\n",file_buffer);

	return 0;
}

