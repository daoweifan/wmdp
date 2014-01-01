/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : xyzModem.c
* By      : Fan Daowei
* Version : V1.0
*
* LICENSING TERMS:
* ---------------
*   WMDP is provided in source form for FREE evaluation, for educational use or for peaceful research.
* If you plan on using  WMDP  in a commercial product you need to contact WM to properly license
* its use in your product. We provide ALL the source code for your convenience and to help you experience
* WMDP.   The fact that the  source is provided does  NOT  mean that you can use it without  paying a
* licensing fee.
*********************************************************************************************************
*/
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include "common/xyzModem.h"
#include "uart.h"
#include "ulp_time.h"
#include "crc.h"

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#define MODEM_SOH 0x01
#define MODEM_STX 0x02
#define MODEM_EOT 0x04
#define MODEM_ACK 0x06
#define MODEM_BSP 0x08
#define MODEM_NAK 0x15
#define MODEM_CAN 0x18
#define MODEM_EOF 0x1A		/* ^Z for DOS officionados */

#define USE_YMODEM_LENGTH

enum {
	false,
	true
};

typedef int cyg_int32;

/* Data & state local to the protocol */
static struct {
	unsigned char pkt[1024], *bufp;
	unsigned char blk, cblk, crc1, crc2;
	unsigned char next_blk; /* Expected block */
	int len, mode, total_retries;
	int total_SOH, total_STX, total_CAN;
	char crc_mode, at_eof, tx_ack;
#ifdef USE_YMODEM_LENGTH
	unsigned long file_length, read_length;
#endif
} xyz;

#define xyzModem_CHAR_TIMEOUT            4000   /* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3     /* Wait for 3 MODEM_CAN before quitting */

/* private variables */
static uart_bus_t * puart;

/* private function declairation */
static int CYGACC_COMM_IF_GETC_TIMEOUT (char *c);
static void CYGACC_COMM_IF_PUTC (char y);
static char parse_num (char *s, unsigned long *val, char **es, char *delim);
static void xyzModem_flush (void);


static int CYGACC_COMM_IF_GETC_TIMEOUT (char *c)
{
#define DELAY 20
	unsigned long counter = 0;
	while (!puart->poll() && (counter < xyzModem_CHAR_TIMEOUT * 1000 / DELAY)) {
		udelay (DELAY);
		counter++;
	}
	if (puart->poll()) {
		*c = puart->getchar();
		return 1;
	}
	return 0;
}

static void CYGACC_COMM_IF_PUTC (char y)
{
	puart->putchar(y);
}

/* Validate a hex character */
inline static char _is_hex (char c)
{
	return (((c >= '0') && (c <= '9')) ||
		((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
inline static int _from_hex (char c)
{
	int ret = 0;

	if ((c >= '0') && (c <= '9')) {
		ret = (c - '0');
	} else if ((c >= 'a') && (c <= 'f')) {
		ret = (c - 'a' + 0x0a);
	} else if ((c >= 'A') && (c <= 'F')) {
		ret = (c - 'A' + 0x0A);
	}
	return ret;
}

/* Convert a character to lower case */
inline static char _tolower (char c)
{
	if ((c >= 'A') && (c <= 'Z')) {
		c = (c - 'A') + 'a';
	}
	return c;
}

/* Parse (scan) a number */
static char parse_num (char *s, unsigned long *val, char **es, char *delim)
{
	char first = true;
	int radix = 10;
	char c;
	unsigned long result = 0;
	int digit;

	while (*s == ' ')
		s++;
	while (*s) {
		if (first && (s[0] == '0') && (_tolower (s[1]) == 'x')) {
			radix = 16;
			s += 2;
		}
		first = false;
		c = *s++;
		if (_is_hex (c) && ((digit = _from_hex (c)) < radix)) {
			/* Valid digit */
			result = (result * radix) + digit;
		} else {
			if (delim != (char *) 0) {
				/* See if this character is one of the delimiters */
				char *dp = delim;
				while (*dp && (c != *dp))
					dp++;
				if (*dp)
					break;		/* Found a good delimiter */
		}
		return false;		/* Malformatted number */
		}
	}
	*val = result;
	if (es != (char **) 0) {
		*es = s;
	}
	return true;
}

/* Wait for the line to go idle */
static void xyzModem_flush (void)
{
	int res;
	char c;
	while (1) {
		res = CYGACC_COMM_IF_GETC_TIMEOUT (&c);
		if (!res)
			return;
	}
}

static int xyzModem_get_hdr(void)
{
	char c;
	int res;
	char hdr_found = false;
	int i, can_total, hdr_chars;
	unsigned short cksum;

	/* Find the start of a header */
	can_total = 0;
	hdr_chars = 0;

	if (xyz.tx_ack) {
		udelay(1000);
		CYGACC_COMM_IF_PUTC (MODEM_ACK);
		xyz.tx_ack = false;
	}
	while (!hdr_found) {
		res = CYGACC_COMM_IF_GETC_TIMEOUT (&c);
		if (res) {
			hdr_chars++;
			switch (c) {
			case MODEM_SOH: // 128 package size
				xyz.total_SOH++;
				hdr_found = true;
				break;
			case MODEM_STX: // 1024 package size
				xyz.total_STX++;
				hdr_found = true;
				break;
			case MODEM_CAN: // canceled 
				xyz.total_CAN++;
				if (++can_total == xyzModem_CAN_COUNT) {
					return xyzModem_cancel;
				} else {
					/* Wait for multiple MODEM_CAN to avoid early quits */
					break;
				}
			case MODEM_EOT: // end of transfer
				if (hdr_chars == 1) {
					udelay(800);
					CYGACC_COMM_IF_PUTC (MODEM_ACK);
					return xyzModem_eof;
				}
			default:
				break;
			}
		} else {
			/* Data stream timed out */
			xyzModem_flush ();	/* Toss any current input */
			CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
			return xyzModem_timeout;
		}
	}

	/* Header found, now read the package number and it's complement*/
	res = CYGACC_COMM_IF_GETC_TIMEOUT ((char *) &xyz.blk);
	if (!res) {
		return xyzModem_timeout;
	}
	res = CYGACC_COMM_IF_GETC_TIMEOUT ((char *) &xyz.cblk);
	if (!res) {
		return xyzModem_timeout;
	}

	/* receive data block, 128 bytes or 1024 bytes*/
	xyz.len = (c == MODEM_SOH) ? 128 : 1024;
	xyz.bufp = xyz.pkt;
	for (i = 0; i < xyz.len; i++) {
		res = CYGACC_COMM_IF_GETC_TIMEOUT (&c);
		if (res) {
			xyz.pkt[i] = c;
		} else {
			return xyzModem_timeout;
		}
	}

	/* receive the crc */
	res = CYGACC_COMM_IF_GETC_TIMEOUT ((char *) &xyz.crc1);
	if (!res) {
		return xyzModem_timeout;
	}
	if (xyz.crc_mode) {
		res = CYGACC_COMM_IF_GETC_TIMEOUT ((char *) &xyz.crc2);
		if (!res) {
			return xyzModem_timeout;
		}
	}

	/* Validate the message, Verify the packet number and it's complement */
	if ((xyz.blk ^ xyz.cblk) != (unsigned char) 0xFF) {
		xyzModem_flush ();
		return xyzModem_frame;
	}

	/* Verify checksum/CRC */
	if (xyz.crc_mode) {
		cksum = cyg_crc16 (xyz.pkt, xyz.len);
		if (cksum != ((xyz.crc1 << 8) | xyz.crc2)) {
			return xyzModem_cksum;
		}
	} else {
		cksum = 0;
		for (i = 0; i < xyz.len; i++) {
			cksum += xyz.pkt[i];
		}
		if (xyz.crc1 != (cksum & 0xFF)) {
			return xyzModem_cksum;
		}
	}

	// udelay(800);
	// CYGACC_COMM_IF_PUTC (MODEM_ACK);
	/* If we get here, the message passes [structural] muster */
	return 0;
}

void xyzModem_init(uart_bus_t *p)
{
	puart = p;
}

int xyzModem_stream_open (connection_info_t * info, int *err)
{
	int stat = 0;
	int retries = xyzModem_MAX_RETRIES;
	// int crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

#ifdef xyzModem_zmodem
	if (info->mode == xyzModem_zmodem) {
		*err = xyzModem_noZmodem;
		return -1;
	}
#endif

	/* clear uart receive buffer */
	while(puart->poll())
		puart->getchar();

	xyz.len = 0;
	xyz.crc_mode = true;
	xyz.at_eof = false;
	xyz.tx_ack = false;
	xyz.mode = info->mode;
	xyz.total_retries = 0;
	xyz.total_SOH = 0;
	xyz.total_STX = 0;
	xyz.total_CAN = 0;
	#ifdef USE_YMODEM_LENGTH
	xyz.read_length = 0;
	xyz.file_length = 0;
	#endif

	CYGACC_COMM_IF_PUTC ((xyz.crc_mode ? 'C' : MODEM_NAK));

	if (xyz.mode == xyzModem_xmodem) {
		/* X-modem doesn't have an information header - exit here */
		xyz.next_blk = 1;
		return 0;
	}

	while (retries-- > 0) {
		stat = xyzModem_get_hdr();
		if (stat == 0) {
			/* Y-modem file information header */
			if (xyz.blk == 0) {

				#ifdef USE_YMODEM_LENGTH
				if (info->filename) {
					/* record file name */
					int i = 0;
					do {
						info->filename[i] = *xyz.bufp;
						xyz.bufp ++;
						i++;
					} while(*xyz.bufp);
					info->filename[i] = *xyz.bufp;
					xyz.bufp ++;
				} else {
					/* skip filename */
					while(*xyz.bufp++);
				}
				/* get the file length */
				parse_num ((char *) xyz.bufp, &xyz.file_length, NULL, " ");
				info->filesize = xyz.file_length;
				#endif
			}
			udelay(2000);
			CYGACC_COMM_IF_PUTC(MODEM_ACK);
			xyz.tx_ack = false;
			xyz.next_blk = 1;
			xyz.len = 0;
			return 0;
		} else if (stat == xyzModem_timeout) {
			// if (--crc_retries <= 0)
				// xyz.crc_mode = false;
			CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
			CYGACC_COMM_IF_PUTC ((xyz.crc_mode ? 'C' : MODEM_NAK));
			xyz.total_retries++;
		}
		if (stat == xyzModem_cancel) {
			break;
		}
	}
	*err = stat;

	return -1;
}

int xyzModem_stream_read (char *buf, int size, int *err)
{
	int stat, total, len;
	int retries;

	total = 0;
	stat = xyzModem_cancel;
	/* Try and get 'size' bytes into the buffer */
	while (!xyz.at_eof && (size > 0)) {
		if (xyz.len == 0) {
			retries = xyzModem_MAX_RETRIES;
			while (retries-- > 0) {
				stat = xyzModem_get_hdr ();

				/* get a package successfully */
				if (stat == 0) {
					if (xyz.blk == xyz.next_blk) {
						xyz.tx_ack = true;
						xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

						if (xyz.mode == xyzModem_xmodem || xyz.file_length == 0) {
							/* Data blocks can be padded with ^Z (MODEM_EOF) characters */
							/* This code tries to detect and remove them */
							if ((xyz.bufp[xyz.len - 1] == MODEM_EOF) &&
							(xyz.bufp[xyz.len - 2] == MODEM_EOF) &&
							(xyz.bufp[xyz.len - 3] == MODEM_EOF))
							{
								while (xyz.len && (xyz.bufp[xyz.len - 1] == MODEM_EOF)) {
									xyz.len--;
								}
							}
						}

						#ifdef USE_YMODEM_LENGTH
						/*
						* See if accumulated length exceeds that of the file.
						* If so, reduce size (i.e., cut out pad bytes)
						* Only do this for Y-modem (and Z-modem should it ever
						* be supported since it can fall back to Y-modem mode).
						*/
						if (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length) {
							xyz.read_length += xyz.len;
							if (xyz.read_length > xyz.file_length) {
								xyz.len -= (xyz.read_length - xyz.file_length);
							}
						}
						#endif
						// udelay(2000);
						// CYGACC_COMM_IF_PUTC (MODEM_ACK);
						// xyz.tx_ack = false;
						break;
					} else if (xyz.blk == ((xyz.next_blk - 1) & 0xFF)) {
						/* Just re-MODEM_ACK this so sender will get on with it */
						udelay(2000);
						CYGACC_COMM_IF_PUTC (MODEM_ACK);
						continue;	/* Need new header */
					} else {
						stat = xyzModem_sequence;
					}
				}

				/* y-modem transfer is cancelled */
				if (stat == xyzModem_cancel) {
					break;
				}

				/* end of file transfer */
				if (stat == xyzModem_eof) {
					// CYGACC_COMM_IF_PUTC (MODEM_ACK);
					if (xyz.mode == xyzModem_ymodem) {
						CYGACC_COMM_IF_PUTC ((xyz.crc_mode ? 'C' : MODEM_NAK));
						xyz.total_retries++;
						stat = xyzModem_get_hdr(); //get the last all zero package
						udelay(800);
						CYGACC_COMM_IF_PUTC (MODEM_ACK);
					}
					xyz.at_eof = true;
					break;
				}
				CYGACC_COMM_IF_PUTC ((xyz.crc_mode ? 'C' : MODEM_NAK));
				xyz.total_retries++;
			}
			if (stat < 0) {
				*err = stat;
				xyz.len = -1;
				return total;
			}
		}

		/* Don't "read" data from the MODEM_EOF protocol package */
		if (!xyz.at_eof) {
			len = xyz.len;
			if (size < len)
				len = size;
			memcpy(buf, xyz.bufp, len);
			size -= len;
			buf += len;
			total += len;
			xyz.len -= len;
			xyz.bufp += len;
		}
	}
	return total;
}

void xyzModem_stream_close (int *err)
{
	diag_printf
		("xyzModem - %s mode, %d(MODEM_SOH)/%d(MODEM_STX)/%d(MODEM_CAN) packets, %d retries\n",
		xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
		xyz.total_CAN, xyz.total_retries);
}

/* Need to be able to clean out the input buffer, so have to take the getc */
void xyzModem_stream_terminate (char abort, int (*getc) (void))
{
	if (abort) {
		switch (xyz.mode) {
		case xyzModem_xmodem:
		case xyzModem_ymodem:
			/* The X/YMODEM Spec seems to suggest that multiple MODEM_CAN followed by an equal */
			/* number of Backspaces is a friendly way to get the other end to abort. */
			CYGACC_COMM_IF_PUTC (MODEM_CAN);
			CYGACC_COMM_IF_PUTC (MODEM_CAN);
			CYGACC_COMM_IF_PUTC (MODEM_CAN);
			CYGACC_COMM_IF_PUTC (MODEM_CAN);
			CYGACC_COMM_IF_PUTC (MODEM_BSP);
			CYGACC_COMM_IF_PUTC (MODEM_BSP);
			CYGACC_COMM_IF_PUTC (MODEM_BSP);
			CYGACC_COMM_IF_PUTC (MODEM_BSP);
			/* Now consume the rest of what's waiting on the line. */
			xyzModem_flush ();
			xyz.at_eof = true;
			break;
#ifdef xyzModem_zmodem
		case xyzModem_zmodem:
			/* Might support it some day I suppose. */
			break;
#endif
		}
	} else {
		/*
		* Consume any trailing crap left in the inbuffer from
		* previous recieved blocks. Since very few files are an exact multiple
		* of the transfer block size, there will almost always be some gunk here.
		* If we don't eat it now, RedBoot will think the user typed it.
		*/
		while ((*getc)() > -1);
		/*
		* Make a small delay to give terminal programs like minicom
		* time to get control again after their file transfer program
		* exits.
		*/
		CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	}
}

char * xyzModem_error (int err)
{
	switch (err) {
	case xyzModem_access:
		return "Can't access file";
	case xyzModem_noZmodem:
		return "Sorry, zModem not available yet";
	case xyzModem_timeout:
		return "Timed out";
	case xyzModem_eof:
		return "End of file";
	case xyzModem_cancel:
		return "Cancelled";
	case xyzModem_frame:
		return "Invalid framing";
	case xyzModem_cksum:
		return "CRC/checksum error";
	case xyzModem_sequence:
		return "Block sequence error";
	default:
		return "Unknown error";
	}
}

