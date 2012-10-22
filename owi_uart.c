
#include	<avr\interrupt.h>
#include	<inttypes.h>
#include	<string.h>

#include	"main.h"
#include	"owi_uart.h"


ISR(USART_RXC_vect){
	unsigned char c;
	if(UCSRA&_BV(FE)){
		c=UDR;
		owi.err=1;
		owi.bsy=0;
		}
	else{
		c=UDR;
		if(UBRRL==OWI_UBRR_9600){
			if(c==0xf0){
				owi.presence=0;
				}
			else{
				owi.presence=1;
				UBRRL=OWI_UBRR_115200;
				owi.r_bits=0;
				owi.bsy=0;
				}
			}
		else{
			owi.r_bits++;
			owi.r_byte>>=1;
			if(c==0xff){
				owi.r_byte|=0x80;
				}
			if(owi.r_bits==owi.r_len){
				c=owi.r_len;
				while(c&7){
					owi.r_byte>>=1;
					c++;
					}
				owi.buff[owi.r_index]=owi.r_byte;
				owi.r_index=0;
				owi.r_bits=0;
				owi.bsy=0;
				}
			else{
				if((owi.r_bits&7) == 0){
					owi.buff[owi.r_index++]=owi.r_byte;
					}
				}
			}
		}
	}


ISR(USART_UDRE_vect){
	if(UBRRL==OWI_UBRR_9600){
		UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(RXCIE);
		UDR=0xf0;
		owi.t_bits=0;
		}
	else{
		if(owi.t_bits==0){
			owi.t_byte=owi.buff[0];
			owi.t_index=0;
			}
		if(owi.t_bits==owi.t_len){
			owi.t_bits=0;
			UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(RXCIE);
			}
		else{
			if(owi.t_byte&1){
				UDR=0xff;
				}
			else{
				UDR=0x00;
				}
			owi.t_byte>>=1;
			owi.t_bits++;
			if((owi.t_bits&7) == 0){
				owi.t_index++;
				owi.t_byte=owi.buff[owi.t_index];
				}
			}
		}
	}


void owi_scan(void){
	unsigned char c;
	if(owi.bsy)	return;
	if((owi.state&0x0f)==0){
		owi.err=0;
		owi.bsy=1;
		owi.state++;
		UBRRL=OWI_UBRR_9600;
		UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(UDRIE)|_BV(RXCIE);
		}
	else if((owi.state&0x0f)==1){
		if(owi.err || (owi.presence==0)){
			owi.state=0;
			}
		else{
			owi.state++;
			}
		}
	else if(owi.state==2){
		owi.bitindex=1;
		owi.bitmask=1;
		owi.nd=0;
		owi.bitpattern_pointer=owi.bitpattern;
		owi_byte(OWI_ROM_SEARCH);
		owi.state++;
		}
	else if(owi.state==3){
		owi_bits(3,2);
		owi.state++;
		}
	else if(owi.state==4){
		c=owi.buff[0]&0x03;
		if(c==0b11){
			owi.state=0;
			}
		else if(c==0b10){
			(*owi.bitpattern_pointer)&=~owi.bitmask;
			}
		else if(c==0b01){
			(*owi.bitpattern_pointer)|=owi.bitmask;
			}
		else{
			if(owi.bitindex==owi.ld){
				(*owi.bitpattern_pointer)|=owi.bitmask;
				}
			else if(owi.bitindex>owi.ld){
				(*owi.bitpattern_pointer)&=~owi.bitmask;
				owi.nd=owi.bitindex;
				}
			else if(((*owi.bitpattern_pointer)&owi.bitmask)	==0){
				owi.nd=owi.bitindex;
				}
			}
		if((*owi.bitpattern_pointer)&owi.bitmask){
			owi_bit(1);
			}
		else{
			owi_bit(0);
			}
		owi.state++;
		}
	else if(owi.state==5){
		owi.bitmask<<=1;
		if(owi.bitmask==0){
			owi.bitpattern_pointer++;
			owi.bitmask=1;
			}
		owi.bitindex++;
		if(owi.bitindex>64){
			if(OWI_CheckRomCRC(owi.bitpattern)==OWI_CRC_OK){
				memcpy(owi.dev[owi.devs],owi.bitpattern,8);
				owi.devs++;
				owi.ld=owi.nd;
				if(owi.nd){
					owi.state=0;
					}
				else{
					owi.state=0x10;
					}
				}
			else{
				owi.state=0;
				}
			}
		else{
			owi.state=3;
			}
		}
	else if(owi.state==0x12){
		owi_byte(OWI_ROM_SKIP);
		owi.state++;
		}
	else if(owi.state==0x13){
		owi_byte(OWI_CONVERT_T);
		owi.state++;
		}
	else if(owi.state==0x14){
		owi_bit(1);
		owi.state++;
		}
	else if(owi.state==0x15){
		if(owi.buff[0]&1){
			owi.devs_index=0;
			owi.state=0x20;
			}
		else{
			owi.state--;
			}
		}
	else if(owi.state==0x22){
		owi_byte(OWI_ROM_MATCH);
		owi.state++;
		}
	else if(owi.state==0x23){
		owi_byte8w(owi.dev[owi.devs_index]);
		owi.state++;
		}
	else if(owi.state==0x24){
		owi_byte(OWI_RAM_READ);
		owi.state++;
		}
	else if(owi.state==0x25){
		owi_byte9r();
		owi.state++;
		}
	else if(owi.state==0x26){
		if(OWI_CheckRamCRC(owi.buff)==OWI_CRC_OK){
			owi.t[owi.devs_index]=*((int*)owi.buff);
			owi.devs_index++;
			}
		if(owi.devs_index>=owi.devs){
			owi.state=0;
			owi.devs=0;
			}
		else{
			owi.state=0x20;
			}
		}
	}


void owi_init(void){
	UCSRA=_BV(U2X);
	UCSRB=0;
	owi.bsy=0;
	owi.err=0;
	owi.presence=0;
	owi.state=0;
	}


void owi_byte8r(void){
	memset(owi.buff,0xff,8);
	owi.t_len=64;
	owi.r_len=64;
	owi.t_bits=0;
	owi.r_bits=0;
	owi.bsy=1;
	UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(UDRIE)|_BV(RXCIE);
	}

void owi_byte9r(void){
	memset(owi.buff,0xff,9);
	owi.t_len=72;
	owi.r_len=72;
	owi.t_bits=0;
	owi.r_bits=0;
	owi.bsy=1;
	UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(UDRIE)|_BV(RXCIE);
	}

void owi_byte8w(unsigned char *p){
	memcpy(owi.buff,p,8);
	owi.t_len=64;
	owi.r_len=64;
	owi.t_bits=0;
	owi.r_bits=0;
	owi.bsy=1;
	UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(UDRIE)|_BV(RXCIE);
	}


void owi_byte(unsigned char	byte){
	owi.buff[0]=byte;
	owi.t_len=8;
	owi.r_len=8;
	owi.t_bits=0;
	owi.r_bits=0;
	owi.bsy=1;
	UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(UDRIE)|_BV(RXCIE);
	}


void owi_bit(unsigned char bit){
	owi.t_len=1;
	owi.r_len=1;
	owi.t_bits=0;
	owi.r_bits=0;
	owi.buff[0]=bit;
	owi.bsy=1;
	UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(UDRIE)|_BV(RXCIE);
	}


void owi_bits(unsigned char	byte,unsigned char n){
	owi.t_len=n;
	owi.r_len=n;
	owi.buff[0]=byte;
	owi.t_bits=0;
	owi.r_bits=0;
	owi.bsy=1;
	UCSRB=_BV(RXEN)|_BV(TXEN)|_BV(UDRIE)|_BV(RXCIE);
	}




/*!	\brief	Compute	the	CRC8 value of a	data set.
 *	This function will compute the CRC8	or DOW-CRC of inData using seed
 *	as inital value	for	the	CRC.
 *	\param	inData	One	byte of	data to	compute	CRC	from.
 *	\param	seed	The	starting value of the CRC.
 *	\return	The	CRC8 of	inData with	seed as	initial	value.
 *	\note	Setting	seed to	0 computes the crc8	of the inData.
 *	\note	Constantly passing the return value	of this	function
 *			As the seed	argument computes the CRC8 value of	a
 *			longer string of data.
 */

unsigned char OWI_ComputeCRC8(unsigned char	inData,	unsigned char seed)
{
	unsigned char bitsLeft;
	unsigned char temp;

	for	(bitsLeft =	8; bitsLeft	> 0; bitsLeft--)
	{
		temp = ((seed ^	inData)	& 0x01);
		if (temp ==	0)
		{
			seed >>= 1;
		}
		else
		{
			seed ^=	0x18;
			seed >>= 1;
			seed |=	0x80;
		}
		inData >>= 1;
	}
	return seed;
}


/*!	\brief	Compute	the	CRC16 value	of a data set.
 *
 *	This function will compute the CRC16 of	inData using seed
 *	as inital value	for	the	CRC.
 *
 *	\param	inData	One	byte of	data to	compute	CRC	from.
 *
 *	\param	seed	The	starting value of the CRC.
 *
 *	\return	The	CRC16 of inData	with seed as initial value.
 *
 *	\note	Setting	seed to	0 computes the crc16 of	the	inData.
 *
 *	\note	Constantly passing the return value	of this	function
 *			As the seed	argument computes the CRC16	value of a
 *			longer string of data.
 */
unsigned int OWI_ComputeCRC16(unsigned char	inData,	unsigned int seed)
{
	unsigned char bitsLeft;
	unsigned char temp;

	for	(bitsLeft =	8; bitsLeft	> 0; bitsLeft--)
	{
		temp = ((seed ^	inData)	& 0x01);
		if (temp ==	0)
		{
			seed >>= 1;
		}
		else
		{
			seed ^=	0x4002;
			seed >>= 1;
			seed |=	0x8000;
		}
		inData >>= 1;
	}
	return seed;
}


/*!	\brief	Calculate and check	the	CRC	of a 64	bit	ROM	identifier.
 *
 *	This function computes the CRC8	value of the first 56 bits of a
 *	64 bit identifier. It then checks the calculated value against the
 *	CRC	value stored in	ROM.
 *
 *	\param	romvalue	A pointer to an	array holding a	64 bit identifier.
 *
 *	\retval	OWI_CRC_OK		The	CRC's matched.
 *	\retval	OWI_CRC_ERROR	There was a	discrepancy	between	the	calculated and the stored CRC.
 */
unsigned char OWI_CheckRomCRC(unsigned char	* romValue)
{
	unsigned char i;
	unsigned char crc8 = 0;

	for	(i = 0;	i <	7; i++)
	{
		crc8 = OWI_ComputeCRC8(*romValue, crc8);
		romValue++;
	}
	if (crc8 ==	(*romValue))
	{
		return OWI_CRC_OK;
	}
	return OWI_CRC_ERROR;
}

unsigned char OWI_CheckRamCRC(unsigned char	* romValue)
{
	unsigned char i;
	unsigned char crc8 = 0;

	for	(i = 0;	i <	8; i++)
	{
		crc8 = OWI_ComputeCRC8(*romValue, crc8);
		romValue++;
	}
	if (crc8 ==	(*romValue))
	{
		return OWI_CRC_OK;
	}
	return OWI_CRC_ERROR;
}