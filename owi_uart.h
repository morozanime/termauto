
/****************************************************************************
 ROM commands
****************************************************************************/
#define		OWI_ROM_READ	0x33	//!< READ ROM command code.
#define		OWI_ROM_SKIP	0xcc	//!< SKIP ROM command code.
#define		OWI_ROM_MATCH	0x55	//!< MATCH ROM command code.
#define		OWI_ROM_SEARCH	0xf0	//!< SEARCH	ROM	command	code.


/****************************************************************************
 Return	codes
****************************************************************************/
#define		OWI_ROM_SEARCH_FINISHED		0x00	//!< Search	finished return	code.
#define		OWI_ROM_SEARCH_FAILED		0xff	//!< Search	failed return code.


/****************************************************************************
 UART patterns
****************************************************************************/
#define		OWI_UART_WRITE1		0xff	//!< UART Write	1 bit pattern.
#define		OWI_UART_WRITE0		0x00	//!< UART Write	0 bit pattern.
#define		OWI_UART_READ_BIT	0xff	//!< UART Read bit pattern.
#define		OWI_UART_RESET		0xf0	//!< UART Reset	bit	pattern.
#define		OWI_CRC_OK			0x00	//!< CRC check succeded
#define		OWI_CRC_ERROR		0x01	//!< CRC check failed


unsigned char OWI_ComputeCRC8(unsigned char	inData,	unsigned char seed);
unsigned int OWI_ComputeCRC16(unsigned char	inData,	unsigned int seed);
unsigned char OWI_CheckRomCRC(unsigned char	* romValue);
unsigned char OWI_CheckRamCRC(unsigned char	*);
void owi_init(void);
void owi_scan(void);
void owi_bit(unsigned char);
void owi_bits(unsigned char,unsigned char);
void owi_byte(unsigned char);
void owi_byte8r(void);
void owi_byte9r(void);
void owi_byte8w(unsigned char*);

#define		OWI_UBRR_9600		103
#define		OWI_UBRR_115200		8
#define		OWI_CONVERT_T		0x44
#define		OWI_RAM_READ		0xBE

struct {
	unsigned char t_bits;
	unsigned char t_len;
	unsigned char t_byte;
	unsigned char t_index;
	unsigned char buff[10];
	unsigned char r_bits;
	unsigned char r_len;
	unsigned char r_byte;
	unsigned char r_index;
//	  unsigned char	r_buff[10];
	unsigned char nd;
	unsigned char ld;
	unsigned char bitindex;
	unsigned char bitmask;
	unsigned char *bitpattern_pointer;
	unsigned char bitpattern[8];
	unsigned char state;
	unsigned char dev[8][8];
	unsigned char devs;
	unsigned char devs_index;
	signed int t[8];
	unsigned char err:1;
	unsigned char bsy:1;
	unsigned char presence:1;
	} owi;