#include "mbed.h"
#include "ArduinoMotorShield.hpp"

/****************************************************************************
 ** command table constants & typedefs
 ****************************************************************************/
#define CMD_NAME_LEN	32
#define CMD_ARG_LEN	32
#define CMD_HELP_LEN	80

typedef int(*CMD_PTR_T)(const char *);

typedef struct {
    char cmd_name[CMD_NAME_LEN];
    CMD_PTR_T cmd;
    char help[CMD_HELP_LEN];
} CMD_TABLE_ENTRY;

/****************************************************************************
 ** program typdefs
 ***************************************************************************/
typedef enum 
{
    MOTOR_FWD, 
    MOTOR_REV,
} MOTOR_DIRECTION;

typedef enum
{
    MOTOR_STOPPED,
    MOTOR_STARTING,
    MOTOR_SPEEDUP,
    MOTOR_RUNNING,
    MOTOR_SLOWDOWN,
} MOTOR_STATE;

typedef enum
{
    LED_OFF = 0,
    LED_ON  = 1
} LED_STATE;

/****************************************************************************
 ** class instantiations 
 ****************************************************************************/
DigitalOut		led2(LED2);
DigitalOut		led1(D7);
InterruptIn		event(D6);
Timer			timer;
Ticker			ticker;
Serial 			sp(USBTX, USBRX);
ArduinoMotorShield	ams;



/****************************************************************************
 ** function prototypes 
 ****************************************************************************/
void led_blink(void);
void counter_read_reset(void);
void led_reset(void);

/****************************************************************************
 ** user command prototypes
 ****************************************************************************/
int get_status(const char *);
int get_pw(const char *);
int set_pw(const char *);
int get_speed(const char *);
int set_speed(const char *);
int get_direction(const char *);
int set_direction(const char *);

/****************************************************************************
 ** command table setup
 ***************************************************************************/
CMD_TABLE_ENTRY cmd_table[] = 
{
    {"pwr?",	&get_pw,	"Print motor power"		},
    {"pwr",	&set_pw,	"Set motor power"		},
    {"dir?",	&get_direction,	"Print motor direction",	},
    {"dir",	&set_direction,	"Set motor direction",		},
    {"help",	NULL, 		"Print some nice help"		},
};

const int n_cmds = sizeof(cmd_table)/sizeof(CMD_TABLE_ENTRY);

/****************************************************************************
 ** variables 
 ****************************************************************************/
char rx_buf[128];

static int		motor_speed	= 0;
static int		motor_power	= 0;
static MOTOR_DIRECTION	motor_dir	= MOTOR_FWD;
static MOTOR_STATE	motor_state	= MOTOR_STOPPED;

static uint32_t		counts		= 0;

/*
*************
*/
int main()
{
    char cmd[CMD_NAME_LEN];
    char arg[CMD_ARG_LEN];

    led1 = LED_ON;

    timer.start();
    event.rise(counter_read_reset);
    event.fall(led_reset);

    sp.baud(115200);
    sp.printf("AVC Test Device Ready\n");

    ams.SetMotorPolarity(ArduinoMotorShield::MOTOR_A, ArduinoMotorShield::MOTOR_FORWARD);
    ams.SetMotorPower(ArduinoMotorShield::MOTOR_A, motor_power);

    while(true) 
    {
	if(gets(rx_buf) != NULL)
	{
	    int fields = sscanf(rx_buf, "%s %s", cmd, arg);
	    for(int i = 0; i < n_cmds; ++i)
	    {
		if(strncmp(cmd, cmd_table[i].cmd_name, strlen(cmd_table[i].cmd_name)) == 0)
		{
		    cmd_table[i].cmd(arg);
		    break;
		}
	    }
	    memset(rx_buf, 0, sizeof(rx_buf));
	    fields = 0;
	}
    }
}


/*
** fetch pulse width
*/
int get_pw(const char *)
{
    printf("pwr = %d\n", motor_power);
    return(0);
}

/*
** set pulse width (motor power)
*/
int set_pw(const char *arg)
{
    int pw_val = 0;
    pw_val = atoi(arg);
    printf("setting pwr = %d\n", pw_val);
    motor_power = pw_val;
    ams.SetMotorPower(ArduinoMotorShield::MOTOR_A, (motor_power/100.0f));
    return(0);
}

/*
** fetch & print motor direction
*/
int get_direction(const char *arg)
{
    char *msg;

    switch(motor_dir)
    {
    case MOTOR_FWD:
	msg = "fwd";
	break;
    case MOTOR_REV:
	msg = "rev";
	break;
    default:
	msg = "invalid";
	break;
    }
    printf("dir = %s\n", msg);
    return(0);
}


/*
** set motor direction
*/
int set_direction(const char *arg)
{
    MOTOR_DIRECTION dir;

    if(strcmp(arg, "fwd") == 0)
    {
	dir = MOTOR_FWD;
    }
    else if(strcmp(arg, "rev") == 0)
    {
	dir = MOTOR_REV;
    }
    else
    {
	return(-1);
    }

    if(motor_dir != dir)
    {
	/* changing direction => stop motor first */
	ams.SetMotorPower(ArduinoMotorShield::MOTOR_A, 0.0f);
    }
    /* set motor to new direction */
    motor_dir = dir;
    ams.SetMotorPolarity(ArduinoMotorShield::MOTOR_A, (ArduinoMotorShield::MotorDirection_e)dir);
    ams.SetMotorPower(ArduinoMotorShield::MOTOR_A, (motor_power / 100.0f));
    printf("Setting motor direction to %d\n", dir);
    return(0);
}

/*
** ISR to capture count -- called on rising edge of detect
*/
void counter_read_reset(void)
{
    counts = timer.read_ms();
    printf("counts = %ld\n", counts);
    timer.reset();
    led1 = LED_ON;
}

/*
** ISR to clear led -- called on falling edge of detect
*/
void led_reset(void)
{
    led1 = LED_OFF;
}
