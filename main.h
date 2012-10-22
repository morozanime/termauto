
#define	F_CPU					11059200UL

extern void	init(void);
extern void	mainloop(void);
extern void	lcd_i2a3(unsigned int);
extern void	lcd_c2a2(unsigned char);
extern void	lcd_c2a3(unsigned char);
extern void	keyscan(void);
extern void	light_set(void);

#define	LCD_DATA_PORT	PORTD
#define	LCD_DATA_SHIFT	2
#define	LCD_RS(X)	((X)?(PORTD|=0x40):(PORTD&=~0x40))
#define	LCD_E(X)	((X)?(PORTD|=0x80):(PORTD&=~0x80))

char lcdbuff[17];


struct {
	unsigned long aver;
	} adc;

struct {
	unsigned char state;
	unsigned char keytime;
	unsigned char mode;
	unsigned char contrast;
	unsigned char light;
	} st;