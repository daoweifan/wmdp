/*
*********************************************************************************************************
*                                                WMDP
*                                          WM Develop Platform
*
*                              (c) Copyright 2010-2014, WM, China
*                                           All Rights Reserved
*
* File    : sim900a.h
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
#ifndef _SIM900A_H
#define _SIM900A_H

enum Power_mode { POWER_OFF=0, POWER_ON, CUT_OFF };
enum Reply { AT_OK=0, AT_FAIL, AT_ERROR, AT_TIMEOUT };

/* C-API */
int gsm_init(const char *device_name);                                   /* Connect the gsm module to device */
int gsm_cmd(const char *cmd);                                            /* Send AT command to modem. Returns AT_OK, AT_FAIL or AT_ERROR */
int gsm_cmd_fmt(const char *fmt, ...);                                   /* Send formatted string command to modem. */
int gsm_cmd_wait(const char *cmd, const char *response, int timeout);    /* Send string and wait for a specific response */
int gsm_cmd_wait_fmt(const char *response, int timeout, char *fmt, ...); /* Send formatted string and wait for response */
int gsm_wait(const char *pattern, int timeout);                          /* wait for a pattern to appear */
int gsm_wait_cpy(const char *pattern, int timeout, char *buf, size_t buf_size); /* Wait and copy rest of the line to a buffer */
static int gsm_ftp_size(void);

void gsm_uart_write(const char *line);
void gsm_set_power_state(enum Power_mode mode);
void gsm_reset_modem(void);
int gsm_is_gps_ready(void);         /* Check if GPS flag is set in GSM */
int gsm_read_line(char *buf, int max_len);
int gsm_read_raw(char *buf, int max_len);
int gsm_gprs_enable(void);
int gsm_gprs_disable(void);

/* gsm ftp function api */
void gsm_ftp_def_init(void);
void gsm_ftp_setip(const char *ip);
void gsm_ftp_setname(const char *name);
void gsm_ftp_setpw(const char *pw);
void gsm_ftp_connect(void);
int gsm_ftp_pre_fread(void);
int gsm_ftp_fread(void *buf, int size, void * fhandler);


/* Internals */
void gsm_line_received(void);
void gsm_setup_io(void);
void gsm_toggle_power_pin(void);
void gsm_enable_voltage(void);
void gsm_disable_voltage(void);
void gsm_set_raw_mode(void);
void gsm_disable_raw_mode(void);
int gsm_is_raw_mode(void);

#endif	/* GSM_H */
