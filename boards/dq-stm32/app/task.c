/*
 * init version, @dusk 
 * 2013,2,26
 */
#include <stdio.h>
#include "os.h"
#include "os_time.h"
#include "device.h"
#include "common/console.h"

device_t uart;
time_t period;

void task_init(void)
{
	// uart = device_find_by_name("uart1");
	// device_write(uart, 0, "this device serial\n", sizeof("this device serial\n"));
	period = time_get(10);
	// console_set_by_name("uart1");
}
// EXPORT_TASK_INIT(task_init)

void task_update(void)
{
	// char ch;
	if (time_left(period) < 0) {
		// if (device_read(uart, 0, &ch, 1))
			// if (ch == '\r')
				// device_write(uart, 0, "\n", 1);
			// else
				// device_write(uart, 0, &ch, 1);
		period = time_get(1000);
		// console_printf("%d\n", period);
		printf("%d", period);
	}
}
// EXPORT_OS_BG_UPDATE(task_update)

