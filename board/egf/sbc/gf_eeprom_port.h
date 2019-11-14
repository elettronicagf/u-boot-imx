#ifndef GF_EEPROM_PORT_H_
#define GF_EEPROM_PORT_H_

#include <common.h>

#define SOM_EEPROM_I2C_BUS_NO 0
#define SOM_EEPROM_I2C_ADDRESS 0x54
#define BOARD_EEPROM_I2C_BUS_NO 0
#define BOARD_EEPROM_I2C_ADDRESS 0
#define DISPLAY_EEPROM_I2C_BUS_NO 1
#define DISPLAY_EEPROM_I2C_ADDRESS 0x56


#undef EEPROM_UPGRADE

#define PRODUCT_CODE_LEN 8
#define FULL_BOARD_CODE_LEN 17
#define SW_ID_LEN 15

#define WID_TABLE_LENGTH 0
#define WID_PROGRAMMING_TABLE_LENGTH 0

#define GFCONFIG_SEPARATOR ";"

#define DEBUG_LEVEL 6

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
