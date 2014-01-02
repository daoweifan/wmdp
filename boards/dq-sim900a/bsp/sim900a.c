/*
 *  Simcom 6320 GSM Driver
 *
 * @author: Daowei Fan
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "wmconfig.h"
#include "interrupt.h"
#include "slre.h"
#include "common/ringbuf.h"
#include "common/circbuf.h"
#include "os_time.h"
#include "sim900a.h"
#include "device.h"


#define BUFF_SIZE	64
#define GSM_CMD_LINE_END "\r\n"
#define TIMEOUT_MS   2000        /* Default timeout 2s */
#define TIMEOUT_HTTP 35000      /* Http timeout, 30s */

enum State {
        STATE_UNKNOWN = 0,
        STATE_OFF = 1,
        STATE_BOOTING,
        STATE_ASK_PIN,
        STATE_WAIT_NETWORK,
        STATE_READY,
        STATE_ERROR,
};
enum CFUN {
        CFUN_0 = 0,
        CFUN_1 = 1,
};
enum GSM_FLAGS {
        HW_FLOW_ENABLED = 0x01,
        SIM_INSERTED    = 0x02,
        GPS_READY       = 0x04,
        GPRS_READY      = 0x08,
        CALL            = 0x10,
        INCOMING_CALL   = 0x20,
        TCP_ENABLED     = 0x40,
};

/* Modem Status */
struct gsm_modem {
	enum Power_mode power_mode;
	enum State state;
	volatile int waiting_reply;
	volatile int raw_mode;
	enum Reply reply;
	int flags;
	enum CFUN cfun;
	int last_sms_index;
	char ap_name[64];
} static gsm = {		/* Initial status */
	.power_mode=POWER_OFF,
	.state = STATE_OFF,
};

/* Handler functions for AT replies*/
static void handle_ok(char *line);
static void handle_fail(char *line);
static void handle_error(char *line);
static void set_hw_flow();
static void parse_cfun(char *line);
static void gpsready(char *line);
static void sim_inserted(char *line);
static void no_sim(char *line);
static void parse_network(char *line);
static void parse_sapbr(char *line);
// static void socket_receive(char *line);
static void pdp_off(char *line);
// static void socket_closed(char *line);
static int32_t gsm_receive_handler(device_t dev, int32_t size);

/* GSM command receive state machine handler, called in uart receive int */
static circbuf_t gsm_rx_buf;
static struct device * dev_gsm;

typedef struct Message Message;
struct Message {
	char *msg;			/* Message string */
	enum State next_state;	/* Atomatic state transition */
	void (*func)();		/* Function to call on message */
};

/* Messages from modem */
static Message urc_messages[] = {
	/* Unsolicited Result Codes (URC messages) */
	{ "RDY",                    .next_state=STATE_BOOTING },
	{ "NORMAL POWER DOWN",      .next_state=STATE_OFF },
	{ "^\\+CPIN: NOT INSERTED", .next_state=STATE_ERROR,        .func = no_sim },
	{ "\\+CPIN: READY",         .next_state=STATE_WAIT_NETWORK, .func = sim_inserted },
	{ "\\+CPIN: SIM PIN",       .next_state=STATE_ASK_PIN,      .func = sim_inserted },
	{ "\\+CFUN:",                                               .func = parse_cfun },
	{ "Call Ready",             .next_state=STATE_READY },
	{ "GPS Ready",                                              .func = gpsready },
	{ "\\+COPS:",                                               .func = parse_network },
	{ "\\+SAPBR:",                                              .func = parse_sapbr },
	{ "\\+PDP: DEACT",                                          .func = pdp_off },
	// { "\\+RECEIVE",      .func = socket_receive },
	/* Return codes */
	{ "OK",                                                     .func = handle_ok },
	{ "FAIL",                                                   .func = handle_fail },
	{ "ERROR",                                                  .func = handle_error },
	{ "\\+CME ERROR",                                           .func = handle_error },
	{ "\\+CMS ERROR",                                           .func = handle_error },                       /* TODO: handle */
	/* SOCKET */
	// { "\\d, CLOSED",    .func = socket_closed },
	{ NULL } /* Table must end with NULL */
};

/* gsm module init function */
int gsm_init(const char *device_name)
{
	/* init gsm hard config */
	dev_gsm = device_find_by_name(device_name);
	device_open(dev_gsm, DEVICE_OFLAG_RDWR);
	device_set_rx_indicate(dev_gsm, gsm_receive_handler);

	buf_init(&gsm_rx_buf, 128);

	/* set gsm apn */
	strcpy(gsm.ap_name, "CMNET");
	
	return 0;
}

static void pdp_off(char *line)
{
	gsm.flags &= ~TCP_ENABLED;
}

static void parse_network(char *line)
{
	char network[64], *p;
	struct slre_cap cap;

	if (slre_match("\"+(.*\"*)", line, strlen(line), &cap, 1)) {
		memcpy(network, cap.ptr, cap.len);
		p = strchr(network,'"');
		if (p) {
			*p = 0x00;
			printf("GSM: Registered to network %s\n", network);
			gsm.state = STATE_READY;
		}
	}
}

#define STATUS_CONNECTING 0
#define STATUS_CONNECTED 1
static const char sapbr_deact[] = "+SAPBR 1: DEACT";

static void parse_sapbr(char *line)
{
	int status;

	/* Example: +SAPBR: 1,1,"10.172.79.111"
	 * 1=Profile number
	 * 1=connected, 0=connecting, 3,4 =closing/closed
	 * ".." = ip addr
	 */
	if (1 == sscanf(line, "+SAPBR: %*d,%d", &status)) {
		switch(status) {
		case STATUS_CONNECTING:
		case STATUS_CONNECTED:
			gsm.flags |= GPRS_READY;
			break;
		default:
			gsm.flags &= ~GPRS_READY;
		}
	} else if (0 == strncmp(line, sapbr_deact, strlen(sapbr_deact))) {
		gsm.flags &= ~GPRS_READY;
	}
}

static void sim_inserted(char *line)
{
	gsm.flags |= SIM_INSERTED;
}

static void no_sim(char *line)
{
	gsm.flags &= ~SIM_INSERTED;
}

static void gpsready(char *line)
{
	gsm.flags |= GPS_READY;
}

static void parse_cfun(char *line)
{
	if (strchr(line,'0')) {
		gsm.cfun = CFUN_0;
	} else if (strchr(line,'1')) {
		gsm.cfun = CFUN_1;
	} else {
		printf("GSM: Unknown CFUN state\n");
	}
}

static void handle_ok(char *line)
{
	gsm.reply = AT_OK;
	gsm.waiting_reply = 0;
}

static void handle_fail(char *line)
{
	gsm.reply = AT_FAIL;
	gsm.waiting_reply = 0;
	printf("GSM: Fail\n");
}

static void handle_error(char *line)
{
	gsm.reply = AT_ERROR;
	gsm.waiting_reply = 0;
	printf("GSM ERROR: %s\n", line);
}

/* Find message matching current line */
static Message *lookup_urc_message(const char *line)
{
	int n;
	//const char *error_msg;
	struct slre_cap caps[4];
	for(n=0; urc_messages[n].msg; n++) {
		if (0 != slre_match(urc_messages[n].msg, line, strlen(line), caps, 4)) {
			return &urc_messages[n];
		}
	}
	return NULL;
}

/**
 * Send AT command to modem.
 * Receives lines from Simcom serial interfaces and parses them.
 * Waits for modem to reply with 'OK', 'FAIL' or 'ERROR'
 * return AT_OK, AT_FAIL or AT_ERROR
 */
int gsm_cmd(const char *cmd)
{
	Message *m;
	int retry, t, i;
	char buf[BUFF_SIZE], c;

	/* Flush buffers */
	gsm_disable_raw_mode();
	buf_flush(&gsm_rx_buf);

	gsm.waiting_reply = 1;
	for(retry=0; retry<3; retry++) {
		i = 0;
		t = time_get(TIMEOUT_MS);       /* Get System timer current value */

		device_write(dev_gsm, 0, cmd, strlen(cmd));
		device_write(dev_gsm, 0, GSM_CMD_LINE_END, strlen(GSM_CMD_LINE_END));

		while(gsm.waiting_reply) {
			/* AT Command Response */
			if (time_left(t) < 0) {
				/* Timeout */
				gsm.reply = AT_TIMEOUT;
				break;
			}
			
			/* check the receive buffer*/
			if (buf_size(&gsm_rx_buf)) {
				// DISABLE_INTERRUPTS();
				buf_pop(&gsm_rx_buf, &c, 1);
				// ENABLE_INTERRUPTS();
				if (c != '\n') {
					if ('\r' == c)
						continue;
					buf[i++] = (char)c;
					if(i==BUFF_SIZE)
						break;
				} else {
					buf[i] = 0;
						/* Skip empty lines */
					if (0 == i) {
						i = 0;
						continue;
					}
					if (0 == strcmp(GSM_CMD_LINE_END, buf)) {
						i = 0;
						continue;
					}
					printf("GSM: %s\n", buf);
					
					/* GSM command and state machine handler. */
					m = lookup_urc_message(buf);
					if (m) {
						if (m->next_state) {
							gsm.state = m->next_state;
						}
						if (m->func) {
							m->func(buf);
						}
					}
					i = 0;
				}
			}
		}
		if (gsm.reply != AT_TIMEOUT)
			break;
	}
	gsm.waiting_reply = 0;

	if (gsm.reply != AT_OK)
		printf("GSM: '%s' failed (%d)\n", cmd, gsm.reply);

	if (retry == 3) {             /* Modem not responding */
		printf("GSM: Modem not responding, RESETING\n");
		// gsm_reset_modem();
		return AT_TIMEOUT;
	}
	return gsm.reply;
}

int gsm_cmd_check(const char *cmd)
{
	int rc;
	printf("GSM CMD: %s\n", cmd);
	rc = gsm_cmd(cmd);
	if (rc) {
		switch(rc) {
		case AT_TIMEOUT:
			printf("TIMEOUT\n");
			break;
		case AT_FAIL:
			printf("Failed\n");
			break;
		case AT_ERROR:
			printf("Returned error\n");
			break;
		}
	}
	return rc;
}

int gsm_cmd_fmt(const char *fmt, ...)
{
	char cmd[256];
	va_list ap;
	va_start( ap, fmt );
	vsnprintf( cmd, 256, fmt, ap );
	va_end( ap );
	return gsm_cmd(cmd);
}

void gsm_set_raw_mode()
{
	gsm.raw_mode = 1;
}
void gsm_disable_raw_mode()
{
	gsm.raw_mode = 0;
}
int gsm_is_raw_mode()
{
	return gsm.raw_mode;
}

/* Wait for specific pattern */
int gsm_wait_cpy(const char *pattern, int timeout, char *line, size_t buf_size)
{
	char buf[256], c;
	int ret, i;
	unsigned int t;
	struct slre_cap caps[4];

	t = time_get(timeout);

	int was_raw_mode = gsm_is_raw_mode();
	gsm_set_raw_mode();

	i = 0;
	ret = AT_OK;
	
	while (1) {
		if (gsm_rx_buf.size > 0) {
			// DISABLE_INTERRUPTS();
			buf_pop(&gsm_rx_buf, &c, 1);
			// ENABLE_INTERRUPTS();
			buf[i++] = c;
			buf[i] = 0;
			if (slre_match(pattern, buf, i, caps, 4) != 0) { //Match
				break;
			}
			if (i == 256) //Buffer full, start new
				i = 0;
			if (c == '\n') //End of line, start new buffer at next char
				i = 0;
		} else {
			if (time_left(t) < 0) {
				ret = AT_TIMEOUT;
				goto WAIT_END;
			} else {
				mdelay(1);
			}
		}
	}

	if (line) {
		strcpy(line, buf);
		i = strlen(line);
		while (1) {
			if (gsm_rx_buf.size > 0) {
				// DISABLE_INTERRUPTS();
				buf_pop(&gsm_rx_buf, &c, 1);
				// ENABLE_INTERRUPTS();
				line[i++] = c;
			}
			if (time_left(t) < 0) {
				ret = AT_TIMEOUT;
				break;
			}
		
		}
	}

WAIT_END:
	if (!was_raw_mode)
		gsm_disable_raw_mode();

	return ret;
}

int gsm_wait(const char *pattern, int timeout)
{
	return gsm_wait_cpy(pattern, timeout, NULL, 0);
}

int gsm_cmd_wait(const char *cmd, const char *response, int timeout)
{
	int r;
	gsm_set_raw_mode();
	gsm_uart_write(cmd);
	gsm_uart_write(GSM_CMD_LINE_END);
	printf("GSM CMD: %s\n", cmd);
	r = gsm_wait(response, timeout);
	gsm_disable_raw_mode();
	return r;
}

int gsm_cmd_wait_fmt(const char *response, int timeout, char *fmt, ...)
{
	char cmd[256];
	va_list ap;
	va_start( ap, fmt );
	vsnprintf( cmd, 256, fmt, ap );
	va_end( ap );
	return gsm_cmd_wait(cmd, response, timeout);
}

/* Send *really* slow. Add inter character delays */
static void slow_send(const char *line)
{
	while(*line) {
		// pgsm->putchar(*line);
		device_write(dev_gsm, 0, line, 1);
		mdelay(10);		/* inter-char delay */
		if ('\r' == *line)
			mdelay(100);		/* Inter line delay */
		line++;
	}
}

void gsm_uart_write(const char *line)
{
	if (!gsm.flags&HW_FLOW_ENABLED)
		slow_send(line);

	// while(*line) {
		// pgsm->putstr(line, strlen(line));
		// line++;
	// }
	device_write(dev_gsm, 0, line, strlen(line));
}

void gsm_uart_write_fmt(const char *fmt, ...)
{
	char cmd[256];
	va_list ap;
	va_start( ap, fmt );
	vsnprintf( cmd, 256, fmt, ap );
	va_end( ap );
	gsm_uart_write(cmd);
}


/**
 * Receives charactor from Simcom serial interfaces and store them into buffer.
 * Called in serial interrupt routine
 */
static int32_t gsm_receive_handler(device_t dev, int32_t size)
{
	char ch[8];
	device_read(dev, 0, ch, size);
	buf_push(&gsm_rx_buf, ch, size);
	return 0;
}

/****************** GSM Module Part API Funciton ************************/
/* Enable GSM module */
void gsm_set_power_state(enum Power_mode mode)
{
	// int status_pin = platform_pio_op(STATUS_PORT, STATUS_PIN, PLATFORM_IO_PIN_GET);
	int status_pin = 1;

	switch(mode) {
	case POWER_ON:
		if (0 == status_pin) {
			// gsm_enable_voltage();
			// gsm_toggle_power_pin();
			// set_hw_flow();
		} else {                    /* Modem already on. Possibly warm reset */
			if (gsm.state == STATE_OFF) {
				gsm_cmd_check("AT+CPIN?");    /* Check PIN, Functionality and Network status */
				gsm_cmd_check("AT+CFUN?");    /* Responses of these will feed the state machine */
				gsm_cmd_check("AT+COPS?");
				gsm_cmd_check("AT+SAPBR=2,1"); /* Query GPRS status */
				// set_hw_flow();
				gsm_cmd_check("ATE0");
				/* We should now know modem's real status */
				/* Assume that gps is ready, there is no good way to check it */
				gsm.flags |= GPS_READY;
			}
		}
		break;
	case POWER_OFF:
		if (1 == status_pin) {
			// gsm_toggle_power_pin();
			// gsm.state = STATE_OFF;
			gsm.flags = 0;
		}
		break;
	case CUT_OFF:
		// gsm_disable_voltage();
		gsm.state = STATE_OFF;
		gsm.flags = 0;
		break;
	}
}


/****************** GSM GPS Part API Funciton ************************/
/* Check if GPS flag is set */
int gsm_is_gps_ready()
{
	if((gsm.flags&GPS_READY) != 0) {
		return true;
	} else {
		return false;
	}
}

/****************** GSM GPRS Part API Funciton ************************/
int gsm_gprs_enable()
{
	int rc;
	/* Wait for network */
	while(gsm.state < STATE_READY)
		mdelay(TIMEOUT_MS);

	/* Check if already enabled, response is parsed in URC handler */
	rc = gsm_cmd("AT+SAPBR=2,1");
	if (rc)
		return rc;

	if (gsm.flags&GPRS_READY)
		return 0;

	rc = gsm_cmd("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
	if (rc)
		return rc;
	rc = gsm_cmd_fmt("AT+SAPBR=3,1,\"APN\",\"%s\"", gsm.ap_name);
	if (rc)
		return rc;
	rc = gsm_cmd("AT+SAPBR=1,1");

	if (AT_OK == rc)
		gsm.flags |= GPRS_READY;
	return rc;
}

int gsm_gprs_disable()
{
	gsm_cmd("AT+SAPBR=2,1");
	if (!(gsm.flags&GPRS_READY))
		return 0; //Was not enabled

	return gsm_cmd("AT+SAPBR=0,1"); //Close Bearer
}

#if 0
/****************** GSM TCP API Funciton ************************/
typedef struct socket {
	int con;
	enum {DISCONNECTED, CONNECTED} state;
	struct circbuf_t buf;
} socket;

socket sockets;


/* Initialize GSM metatable */
static void gsm_tcp_initialize(void)
{
	buf_init(&(sockets.buf), 256);
}

static int gsm_tcp_enable()
{
	int rc = 0;
	while(gsm.state < STATE_READY)
		mdelay(100);

	if (gsm.flags&TCP_ENABLED)
		return 0;

	rc = gsm_cmd("AT+CPIN?");  /* sim card status */
	if (rc)
		return rc;
	rc = gsm_cmd("AT+CSQ");  /* signal quarlity */
	if (rc)
		return rc;
	rc = gsm_cmd("AT+CREG?");  /* check network register */
	if (rc)
		return rc;
	rc = gsm_cmd("AT+CIPSPRT=1"); /* Report '>' */
	if (rc)
		return rc;
	rc = gsm_cmd("AT+CGATT?");
	if (rc)
		return rc;
	rc = gsm_cmd_fmt("AT+CSTT=\"%s\"", gsm.ap_name);
	if (rc)
		return rc;
	rc = gsm_cmd("AT+CIICR");
	if (rc)
		return rc;
	rc = gsm_cmd_wait("AT+CIFSR", "\\d+.\\d+.\\d+.\\d", TIMEOUT_MS);
	if (!rc)
		gsm.flags |= TCP_ENABLED;
	return rc;
}

static void gsm_tcp_end()
{
	gsm_cmd_wait("AT+CIPSHUT", "SHUT OK", TIMEOUT_MS);
}

//address: 
//port:    
enum MODE { TCP, UDP };
static int gsm_tcp_connect(void)
{
	int i,rc;
	int mode = TCP;
	socket *s = 0;

	const char *addr = luaL_checkstring(L,1);
	const int port = luaL_checkinteger(L,2);
	if (3 == lua_gettop(L))
		mode = luaL_checkinteger(L, 3);

	gsm_tcp_initialize();

	if (AT_OK != gsm_tcp_enable()) {
		printf("GSM: Cannot enable GPRS PDP\n");
		gsm_tcp_end();
		return 0;
	}

	/* Find free socket */
	if (sockets.state == DISCONNECTED) {
		s = &sockets;
		s->con = 0;
	}

	if (NULL == s) {
		printf("GSM: No more free sockets\n");
		return 0;
	}

	/* Connect */
	rc = gsm_cmd_fmt("AT+CIPSTART=\"%s\",\"%s\",%d",(TCP==mode)?"TCP":"UDP", "zeontech.com", "8888");
	rc = gsm_wait("CONNECT OK", 2*TIMEOUT_HTTP);
	if (AT_OK != rc) {
		printf("GSM: Connection failed\n");
		return 0;
	}

	/* Prepare socket */
	s->state = CONNECTED;
	printf("Connected\n");
	return 1; // lua_newuserdata pushed alredy to stack
}

static int gsm_tcp_clean(int socket)
{
	sockets.state = DISCONNECTED;
	/* Empty receive buffers */
	buf_free(&(sockets.buf));
	return 0;
}

static int gsm_tcp_read(void)
{
	int size,i;
	char *p;
	socket *s = *(socket **)luaL_checkudata(L, 1, "socket_meta");

	if (!rbuff_is_empty(s->buf)) {
		if (lua_gettop(L)>=2) {     /* Size given */
			size=luaL_checkint(L,2);
			p = malloc(size);
			if (NULL == p) {
				printf("GSM: Out of memory\n");
				return 0;
			}
			i=0;
			while(size) {
				if(rbuff_is_empty(s->buf)) {
					delay_ms(100);
					continue;
				}
				p[i++] = rbuff_pop(s->buf);
				size--;
			}
			lua_pushlstring(L, p, i);
			free(p);
		} else {                    /* No size parameter, read all */
			size = rbuff_len(s->buf);
			p = rbuff_get_raw(s->buf);
			lua_pushlstring(L, p, size);
		}
		return 1;
	}
	return 0;
}

static void socket_receive(char *line)
{
	int n,size,con;
	char c;
	socket *s;

	if(2 == sscanf(line, "+RECEIVE,%d,%d", &con, &size)) {
		gsm_set_raw_mode();
		s = &sockets;

		if ((s->buf->size - rbuff_len(s->buf)) > size) { /* Need more space */
			if (!rbuff_grow(s->buf, size + (s->buf->size - rbuff_len(s->buf)))) {
				printf("GSM: Failed to receive socket. Not enough memory\n");
				goto RECV_END;
			}
		}
		for (n=0; n<size; n++) {
			if (0 == gsm_read_raw(&c, 1)) {
				printf("GSM: Socket read failed\n");
				goto RECV_END;
			}
			rbuff_push(s->buf, c);
		}
	}
RECV_END:
	gsm_disable_raw_mode();
}

static int gsm_tcp_write(lua_State *L)
{
	socket *s = *(socket **)luaL_checkudata(L, 1, "socket_meta");
	size_t size;
	const char *str = luaL_checklstring(L, 2, &size);
	char cmd[64];

	if(DISCONNECTED == s->state)
		return luaL_error(L, "Socket disconnected");
	gsm_set_raw_mode();
	snprintf(cmd, 64, "AT+CIPSEND=%d,%d" GSM_CMD_LINE_END, s->con, size);
	gsm_uart_write(cmd);
	if (AT_OK != gsm_wait(">", TIMEOUT_MS) ) {
		printf("Failed to write socket\n");
		return 0;
	}
	gsm_uart_write(str);
	gsm_disable_raw_mode();
	if (AT_TIMEOUT == gsm_wait("SEND OK", TIMEOUT_HTTP)) {
		printf("GSM:Timeout sending socket data\n");
		return 0;
	}
	lua_pushinteger(L, size);
	return 1;
}

static int gsm_tcp_close(void)
{
	if (s->state != DISCONNECTED) {
		gsm_cmd_wait_fmt("CLOSE OK", TIMEOUT_MS, "AT+CIPCLOSE=0,%d", s->con);
		s->state = DISCONNECTED;
	}
	gsm_tcp_clean(s->con);
	return 0;
}

/* Callback from URC */
void socket_closed(char *line)
{
	int s;
	if (1 == sscanf(line,"%d", &s)) {
		sockets[s].state = DISCONNECTED;
	}
}
#endif
/************* GSM Module debug command **************/
#if 1
#include "command.h"
#include "os.h"

static int cmd_gsm_func(int argc, char *argv[])
{
	const char *usage = { \
		"usage:\n " \
		"gsm init uartx        , map gsm to uart and init uart channel\n " \
		"gsm power on/off      , turn the module on/off\n " \
		"gsm cmd line          , send cmd line to gsm module\n "
		"gsm info              , display the vendor&module info\n " \
		"gsm gprs on/off       , turn gprs on/off\n "
		"gsm gps status        , return gps status\n"
	};
	
	if(argc < 2) {
		printf(usage);
		return 0;
	}

	if (!strcmp(argv[1], "init")) {
		switch (argv[2][4]) {
		case '1':
			gsm_init("uart1");
			printf("Init OK\n");
			break;
		case '2':
			gsm_init("uart2");
			printf("Init OK\n");
			break;
		case '3':
			gsm_init("uart3");
			printf("Init OK\n");
			break;
		default:
			printf("Error Param\n");
			break;
		}
		return 0;
	}

	if (!strcmp(argv[1], "power")) {
		if (argv[2][1] == 'n') {
			gsm_set_power_state(POWER_ON);
		} else if (argv[2][1] == 'f') {
			gsm_set_power_state(POWER_OFF);
		} else {
			printf("Error Para\n");
		}
		return 0;
	}

	if (!strcmp(argv[1], "info")) {
		if (argc == 2) {
			gsm_cmd_check("AT+GSV"); //gsm module id
			gsm_cmd_check("AT+COPS?"); //net work vendor
		} else {
			printf("Error Para\n");
		}
		return 0;
	}

	if (!strcmp(argv[1], "cmd")) {
		if (argc == 3) {
			gsm_cmd(argv[2]);
		} else {
			printf("Error Para\n");
		}
		return 0;
	}

	if (!strcmp(argv[1], "gprs")) {
		if (argv[2][1] == 'n') {
			gsm_gprs_enable();
		} else if (argv[2][1] == 'f') {
			gsm_gprs_disable();
		} else {
			printf("Error Param\n");
		}
		return 0;
	}

	return 0;
}
const cmd_t cmd_gsm = {"gsm", cmd_gsm_func, "GSM Module debug command"};
EXPORT_SHELL_CMD(cmd_gsm)
#endif
