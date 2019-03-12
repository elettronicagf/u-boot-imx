#ifndef GF_EEPROM_PORT_H_
#define GF_EEPROM_PORT_H_

#include <common.h>

#define SOM_EEPROM_I2C_BUS_NO 1
#define SOM_EEPROM_I2C_ADDRESS 0x50
#define BOARD_EEPROM_I2C_BUS_NO 1
#define BOARD_EEPROM_I2C_ADDRESS 0x54

#undef EEPROM_UPGRADE

#define PRODUCT_CODE_LEN 8
#define FULL_BOARD_CODE_LEN 17
#define SW_ID_LEN 15

#define WID_TABLE_LENGTH 0
#define WID_PROGRAMMING_TABLE_LENGTH 23

#define GFCONFIG_SEPARATOR ";"

#define DEBUG_LEVEL 6

/* SOM SW REVISIONS*/
#define REV_WID0510_AA0101 "WID0510_AA01.01"
#define REV_WID0510_AB0101 "WID0510_AB01.01"
#define REV_WID0510_AC0101 "WID0510_AC01.01"
#define REV_WID0510_AE0101 "WID0510_AE01.01"
#define REV_WID0510_AD0101 "WID0510_AD01.01"
#define REV_WID0510_AF0101 "WID0510_AF01.01"
#define REV_WID0510_AG0101 "WID0510_AG01.01"
#define REV_WID0510_AJ0101 "WID0510_AJ01.01"
#define REV_WID0510_AK0101 "WID0510_AK01.01"
#define REV_WID0510_AC0102 "WID0510_AC01.02"
#define REV_WID0510_AE0102 "WID0510_AE01.02"
#define REV_WID0510_AF0102 "WID0510_AF01.02"
#define REV_WID0510_AG0102 "WID0510_AG01.02"
#define REV_WID0510_AJ0102 "WID0510_AJ01.02"
#define REV_WID0510_AK0102 "WID0510_AK01.02"
#define REV_WID0510_AN0101 "WID0510_AN01.01"

/* KIT SW REVISIONS */
#define REV_WID0533_AA0101 "WID0533_AA01.01"
#define REV_WID0533_AB0101 "WID0533_AB01.01"
#define REV_WID0533_BA0101 "WID0533_BA01.01"
#define REV_WID0533_BB0101 "WID0533_BB01.01"
#define REV_WID0533_BB0201 "WID0533_BB02.01"
#define REV_WID0533_BC0101 "WID0533_BC01.01"

#define WID_REV_PGF0533_A01	"WID0533_A"
#define WID_REV_PGF0533_A02	"WID0533_B"

#define PCB_REV_PGF0533_A01	1
#define PCB_REV_PGF0533_A02	2
#define PCB_REV_UNKNOWN		3
#define PCB_REV_NOT_PROGRAMMED 4

#define gf_debug(dbg_level,fmt,args...) \
	if (dbg_level<=DEBUG_LEVEL) printf(fmt, ##args); \
	else (void)0

#define GF_ATTRIBUTES __attribute__((section (".data")))

#define ISSPACE(c)  ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define ISASCII(c)  (((c) & ~0x7f) == 0)
#define ISUPPER(c)  ((c) >= 'A' && (c) <= 'Z')
#define ISLOWER(c)  ((c) >= 'a' && (c) <= 'z')
#define ISALPHA(c)  (ISUPPER(c) || ISLOWER(c))
#define ISDIGIT(c)  ((c) >= '0' && (c) <= '9')

struct wid_translation {
	char product_code[PRODUCT_CODE_LEN + 1];
	char sw_id_code[SW_ID_LEN + 1];
};

struct wid_list_for_eeprom_programming {
	char product_code[FULL_BOARD_CODE_LEN + 1];
	char sw_id_code[SW_ID_LEN + 1];
};

extern struct wid_translation legacy_wid_translation_table[WID_TABLE_LENGTH];

void *gf_memset(void *s, int c, size_t n);
size_t gf_strlen(const char * s);
int gf_strncmp(const char * cs, const char * ct, size_t count);
void gf_strcpy (char dest[], const char src[]);
unsigned long gfstr_toul (char*   nstr, char**  endptr, int base);
void gf_u8tox(u8 i, char *s);
char * gf_strcat (char *dest, const char *src);
void gf_i2c_init(void);
int gf_i2c_set_bus_num(unsigned int bus);
int gf_i2c_probe (u8 chip);
void gf_serial_init(void);
int gf_serial_getc(void);
void gf_som_eeprom_unlock(void);
void gf_som_eeprom_lock(void);
int gf_eeprom_read(u8 address,u16 start_address,u8 * buffer,int len);
int gf_eeprom_write(u8 address,u16 start_address,u8 * buffer,int len);
int gf_read_programmer_file(const char * file_name,char * file_buffer,int buffer_length);

#endif /* GF_EEPROM_PORT_H_ */
