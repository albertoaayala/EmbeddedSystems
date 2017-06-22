#include <includes.h>


/*
*********************************************************************************************************
*                                                CONSTANTS
*********************************************************************************************************
*/

// control task frequency (Hz)
#define RT_FREQ 50

//setpoint parameters
#define SPEED 0.08  // tested up to .12!
#define RADIUS 350
#define CENTER_X 1650
#define CENTER_Y 1350

/*
*********************************************************************************************************
*                                                VARIABLES
*********************************************************************************************************
*/

OS_STK  AppStartTaskStk[APP_TASK_START_STK_SIZE];
// TODO define task stacks
OS_STK DisplayTaskStack[DISPLAY_TASK_STK_SIZE];
OS_STK TouchscreenTaskStack[TOUCHSCREEN_TASK_STK_SIZE];
OS_STK MotorTaskStack[MOTOR_TASK_STK_SIZE];

// control setpoint
double Xpos_set = 1650.0, Ypos_set = 1550.0;

// raw, unfiltered X and Y position of the ball
CPU_INT16U Xpos, Ypos;

// filtered X and Y position of the ball
CPU_INT16U Xposf, Yposf;

CPU_INT08U select = X_DIM;

CPU_INT16U tick;
OS_EVENT *semTouch;
OS_EVENT *semDisp;
OS_EVENT *semMotor;
OS_TMR *dispTimer;
OS_TMR *touchTimer;
OS_TMR *motorTimer;
    //CPU_INT08U  err;


/*
*********************************************************************************************************
*                                            FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppStartTask(void *p_arg);
static  void  AppTaskCreate(void);
// TODO declare function prototypes
static void DisplayTask(void *p_arg);
static void MotorTask(void *p_arg);
static void TouchscreenTask(void *p_arg);

/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.
* Arguments   : none
*********************************************************************************************************
*/

CPU_INT16S  main (void)
{
    CPU_INT08U  err;

    BSP_IntDisAll();                                                    /* Disable all interrupts until we are ready to accept them */
    OSInit();                                                           /* Initialize "uC/OS-II, The Real-Time Kernel"              */

    OSTaskCreateExt(AppStartTask,                                       /* Create the start-up task for system initialization       */
                    (void *)0,
                    (OS_STK *)&AppStartTaskStk[0],
                    APP_TASK_START_PRIO,
                    APP_TASK_START_PRIO,
                    (OS_STK *)&AppStartTaskStk[APP_TASK_START_STK_SIZE-1],
                    APP_TASK_START_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR);
    OSTaskNameSet(APP_TASK_START_PRIO, (CPU_INT08U *)"Start Task", &err);

    OSStart();                                                          /* Start multitasking (i.e. give control to uC/OS-II)       */
	return (-1);                                                        /* Return an error - This line of code is unreachable       */
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppStartTask()' by 'OSTaskCreate()'.
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*               2) Interrupts are enabled once the task start because the I-bit of the CCR register was
*                  set to 0 by 'OSTaskCreate()'.
*********************************************************************************************************
*/
static void touchTimerHandler(void *ptmr, void *p_arg){
    OSSemPost(semTouch);
}

static void motorTimerHandler(void *ptmr, void *p_arg){
    OSSemPost(semMotor);
}

static void dispTimerHandler(void *ptmr, void *p_arg){
    OSSemPost(semDisp);
}

static  void  AppStartTask (void *p_arg)
{
	(void)p_arg;

        CPU_INT08U  err;
	
    BSP_Init();                                                         /* Initialize BSP functions                                 */
    OSStatInit();                                                       /* Determine CPU capacity                                   */
    DispInit();
    //DispClrScr();
    // TODO initialize touchscreen and motors
    touch_init();
    motor_init(MOTOR_X_CHAN);
    motor_init(MOTOR_Y_CHAN);
    semDisp = OSSemCreate(0);
    semTouch = OSSemCreate(0);
    semMotor = OSSemCreate(0);

    touchTimer = OSTmrCreate(0, 10, OS_TMR_OPT_PERIODIC, touchTimerHandler, (void *)0, "Touch Tmr", &err);
    motorTimer = OSTmrCreate(0, 50, OS_TMR_OPT_PERIODIC, motorTimerHandler, (void *)0, "Motor Tmr", &err);
    dispTimer = OSTmrCreate(0, 1000, OS_TMR_OPT_PERIODIC, dispTimerHandler, (void *)0, (INT8U *)"Disp Tmr", &err);

    OSTmrStart(touchTimer, &err);
    OSTmrStart(motorTimer, &err);
    OSTmrStart(dispTimer, &err);

    AppTaskCreate();                                                    /* Create additional user tasks                             */

    while (DEF_TRUE) {
	    OSTimeDlyHMSM(0, 0, 5, 0);
    }
}


/*
*********************************************************************************************************
*                              CREATE ADDITIONAL APPLICATION TASKS
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
	// TODO create tasks
    OSTaskCreateExt(DisplayTask, (void*)0, (OS_STK *)&DisplayTaskStack[0],
                    DISPLAY_TASK_PRIO,
                    DISPLAY_TASK_PRIO,
                    (OS_STK *)&DisplayTaskStack[DISPLAY_TASK_STK_SIZE-1],
                    DISPLAY_TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); //display
    OSTaskCreateExt(TouchscreenTask, (void*)0, (OS_STK *)&TouchscreenTaskStack[0],
                    TOUCHSCREEN_TASK_PRIO,
                    TOUCHSCREEN_TASK_PRIO,
                    (OS_STK *)&TouchscreenTaskStack[TOUCHSCREEN_TASK_STK_SIZE-1],
                    TOUCHSCREEN_TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); //touchscreen
    OSTaskCreateExt(MotorTask, (void*)0, (OS_STK *)&MotorTaskStack[0],
                    MOTOR_TASK_PRIO,
                    MOTOR_TASK_PRIO,
                    (OS_STK *)&MotorTaskStack[MOTOR_TASK_STK_SIZE-1],
                    MOTOR_TASK_STK_SIZE,
                    (void *)0,
                    OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR); // motor
}


static void DisplayTask(void *p_arg) {
    uint8_t ledId = 1;
    CPU_INT08U  err;
    DispStr(0, 0, "CS431 Group 3 Lab 10");
    char buff[100];
    uint16_t uptime = 0;
    while(1){
        OSSemPend(semDisp, 0, &err);
        sprintf(buff, "Uptime: %4d", uptime);
        DispStr(1, 0, buff);
        memset(buff, 0, 100);
        sprintf(buff, "Xpos: %4d", Xpos);
        DispStr(2, 0, buff);
        memset(buff, 0, 100);
        sprintf(buff, "YPos: %4d", Ypos);
        DispStr(3, 0, buff);
        memset(buff, 0, 100);
        LED_Off(0);
        LED_On(ledId++);
        if(ledId > 5) ledId = 1;
        uptime++;
        //OSTimeDlyHMSM(0,0,1,0);
    }
}

static void MotorTask(void *p_arg) {
    CPU_INT08U  err;
    pid_control_t xpid, ypid;
    uint16_t duty_us_x, duty_us_y;
    extrema_t touchscreen;
    build_extrema(&touchscreen, 300, 3000, 300, 3000);
    build_pid_control(&xpid, 0, 0, 0, 0, 0, 0.25, 0.005, 0.05);
    build_pid_control(&ypid, 0, 0, 0, 0, 0, 0.075, 0.005, 0.1);
    while(1){
        OSSemPend(semMotor, 0, &err);
        Xpos_set = CENTER_X + RADIUS * cos(tick * SPEED);
        Ypos_set = CENTER_Y + RADIUS * sin(tick * SPEED);
        tick++;

        update_pid_control(&xpid, Xpos_set, Xpos, 0.05);
        uint16_t newXPosition = Xpos_set+xpid.output;
        duty_us_x = touch_to_duty(&touchscreen, newXPosition, 0);
        update_pid_control(&ypid, Ypos_set, Ypos, 0.05);
        uint16_t newYPosition = Ypos_set+ypid.output;
        duty_us_y = touch_to_duty(&touchscreen, newYPosition, 1);

        // TODO: Convert PID to motor duty cycle (900-2100 us)

        // setMotorDuty is a wrapper function that calls your motor_set_duty
        // implementation in flexmotor.c. The 2nd parameter expects a value
        // between 900-2100 us
        //Set X motor
        motor_set_duty(MOTOR_X_CHAN, duty_us_x);
        //Set Y
        motor_set_duty(MOTOR_Y_CHAN, duty_us_y);

        //OSTimeDlyHMSM(0,0,0,50);
    }
}

static void TouchscreenTask(void *p_arg) {
    CPU_INT08U  err;
    while(1){
        OSSemPend(semTouch, 0, &err);
        if (select == X_DIM) {
            // TODO: read 5 samples from X-dimension and set Xpos as the median
            Xpos = readTouchX(0);
            touch_select_dim(Y_DIM);
            select = Y_DIM;
        } else {
            // TODO: read 5 samples from Y-dimension and set Ypos as the median
            Ypos = readTouchY(0);
            touch_select_dim(X_DIM);
            select = X_DIM;
        }
        //OSTimeDlyHMSM(0,0,0,10);
    }
}


