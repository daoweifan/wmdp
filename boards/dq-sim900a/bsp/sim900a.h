/*
 *  Simcom 6320 GSM Driver
 *
 * @author: Daowei Fan
 */
#ifndef _SIM6320_H
#define _SIM6320_H

enum Power_mode { POWER_OFF=0, POWER_ON, CUT_OFF };
enum Reply { AT_OK=0, AT_FAIL, AT_ERROR, AT_TIMEOUT };

/* C-API */
int gsm_cmd(const char *cmd);                                            /* Send AT command to modem. Returns AT_OK, AT_FAIL or AT_ERROR */
int gsm_cmd_fmt(const char *fmt, ...);                                   /* Send formatted string command to modem. */
int gsm_cmd_wait(const char *cmd, const char *response, int timeout);    /* Send string and wait for a specific response */
int gsm_cmd_wait_fmt(const char *response, int timeout, char *fmt, ...); /* Send formatted string and wait for response */
int gsm_wait(const char *pattern, int timeout);                          /* wait for a pattern to appear */
int gsm_wait_cpy(const char *pattern, int timeout, char *buf, size_t buf_size); /* Wait and copy rest of the line to a buffer */


void gsm_uart_write(const char *line);
void gsm_set_power_state(enum Power_mode mode);
void gsm_reset_modem();
int gsm_is_gps_ready();         /* Check if GPS flag is set in GSM */
int gsm_read_line(char *buf, int max_len);
int gsm_read_raw(char *buf, int max_len);
int gsm_gprs_enable();
int gsm_gprs_disable();

/* Internals */
void gsm_line_received();
void gsm_setup_io();
void gsm_toggle_power_pin();
void gsm_enable_voltage();
void gsm_disable_voltage();
void gsm_set_raw_mode();
void gsm_disable_raw_mode();
int gsm_is_raw_mode();

#endif	/* GSM_H */
