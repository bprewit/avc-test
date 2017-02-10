#include "mbed.h"
#include "ArduinoMotorShield.hpp"

#define CMD_NAME_LEN	32
#define CMD_ARG_LEN	32
#define CMD_HELP_LEN	80

DigitalOut		led(LED2);
Timer			tmr;
Ticker			tkr;
Serial 			sp(USBTX, USBRX);
ArduinoMotorShield	ams;

void blink(void);

char rx_buf[128];

int get_pw(const char *);

typedef int(*CMD_PTR_T)(const char *);

typedef struct {
    char cmd_name[CMD_NAME_LEN];
    // int (*cmd)(const char *);
    CMD_PTR_T cmd;
    char help[CMD_HELP_LEN];
} CMD_TABLE_ENTRY;

CMD_TABLE_ENTRY cmd_table[] = {
    {"pw?", &get_pw, "Return pulse width"},
    {"help", NULL, "Print some nice help"},
};

const int n_cmds = sizeof(cmd_table)/sizeof(CMD_TABLE_ENTRY);

int main()
{
    char cmd[CMD_NAME_LEN];
    char arg[CMD_ARG_LEN];

    sp.baud(115200);
    sp.printf("AVC Test Device Operational\n");

    tkr.attach(blink, 0.250);

    ams.SetMotorPolarity(ArduinoMotorShield::MOTOR_A, ArduinoMotorShield::MOTOR_FORWARD);
    ams.SetMotorPower(ArduinoMotorShield::MOTOR_A, 0.0);

    while (true) {
	if(gets(rx_buf) != NULL)
	{
	    int fields = sscanf(rx_buf, "%s %s", cmd, arg);
	    for(int i = 0; i < n_cmds; ++i)
	    {
		if(strncmp(cmd, cmd_table[i].cmd_name, strlen(cmd_table[i].cmd_name)) == 0)
		{
		    cmd_table[i].cmd(arg);
		}
	    }
	    fields = 0;
	    memset(rx_buf, 0, sizeof(rx_buf));
	}
    }
}

void blink(void)
{
    led = !led;
}

int get_pw(const char *)
{
    printf("pw = 1\n");
    return(0);
}
