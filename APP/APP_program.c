/*
 * APP_program.c
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#include "../LIB/std_types.h"

#include "../MCAL/TMR/TMR_interface.h"
#include "../MCAL/GIE/GIE_interface.h"
#include "../MCAL/TWI/TWI_interface.h"

#include "../HAL/DS1307/DS1307_interface.h"
#include "../HAL/LCD/LCD_interface.h"
#include "../HAL/KPD/KPD_interface.h"

#include "../SERVICES/Alarm_Service/Alarm_Service_interface.h"
#include "../SERVICES/Stopwatch_Service/Stopwatch_Service_interface.h"
#include "../SERVICES/Countdown_Service/Countdown_Service_interface.h"
#include "../SERVICES/Buzzer_Service/Buzzer_Service_interface.h"

#include "APP_interface.h"
#include "APP_config.h"
#include "APP_private.h"


/* ================= Application Pages ================= */

typedef enum
{
    APP_PAGE_CLOCK = 0,
    APP_PAGE_MENU,
    APP_PAGE_CLOCK_SETUP,
    APP_PAGE_SET_TIME,
    APP_PAGE_SET_DATE,
    APP_PAGE_CLOCK_FORMAT,
    APP_PAGE_ALARM,
    APP_PAGE_SET_ALARM,
    APP_PAGE_STOPWATCH,
    APP_PAGE_COUNTDOWN,
    APP_PAGE_SET_COUNTDOWN,
    APP_PAGE_ALARM_TONE,
    APP_PAGE_COUNTDOWN_TONE

} APP_Page_t;


/* ================= Private Variables ================= */

/* Counts Timer1 Compare Matches */
static volatile u8 App_u8TenMsCounter = 0;

/* Counts Pending Seconds */
static volatile u8 App_u8SecondsPending = 0;

/* User Interface State */
static APP_Page_t App_eCurrentPage = APP_PAGE_CLOCK;
static APP_Page_t App_eInputReturnPage = APP_PAGE_CLOCK;

static u8 App_u8MenuSelection = 0;
static u8 App_u8ClockSetupSelection = 0;
static u8 App_u8Clock24HourMode = 1;
static u8 App_u8RingtoneDraft = BUZZER_RINGTONE_NOKIA;
static u8 App_u8RingtoneSource = BUZZER_SERVICE_SOURCE_ALARM;
static u8 App_u8LCDDirty = 1;

/* RTC Cache */
static DS1307_TimeDate_t App_TimeDate;
static u8 App_u8TimeValid = 0;
static u8 App_u8DateValid = 0;

/* Numeric Input Buffer */
static u8 App_au8InputBuffer[10];
static u8 App_u8InputLength = 0;
static u8 App_u8InputTargetLength = 0;
static u8 App_u8InputError = 0;


/* ================= Private Function Prototypes ================= */

static void App_vidTimer1Callback(void);
static void App_vidRefreshRTC(void);
static void App_vidHandleKey(u8 Copy_u8Key);
static void App_vidStartInput(APP_Page_t Copy_ePage,
                              APP_Page_t Copy_eReturnPage,
                              u8 Copy_u8TargetLength);
static void App_vidCommitInput(void);
static void App_vidRenderScreen(void);
static void App_vidWriteText(const char *Copy_pcText);
static void App_vidWriteTwoDigits(u8 Copy_u8Value);
static void App_vidWriteThreeDigits(u8 Copy_u8Value);
static void App_vidWriteFiveDigits(u16 Copy_u16Value);
static void App_vidWriteDisplayHour(u8 Copy_u8Hours);
static void App_vidWriteAmPm(u8 Copy_u8Hours);
static u8 App_u8GetInputDigit(u8 Copy_u8Index);
static u16 App_u16ParseInput(u8 Copy_u8StartIndex, u8 Copy_u8Length);
static u8 App_u8GetDayOfWeek(u8 Copy_u8Date,
                              u8 Copy_u8Month,
                              u8 Copy_u8Year);
static void App_vidDrawClock(void);
static void App_vidDrawMenu(void);
static void App_vidDrawClockSetup(void);
static void App_vidDrawClockFormat(void);
static void App_vidDrawRingtone(void);
static void App_vidDrawInput(void);
static void App_vidDrawAlarm(void);
static void App_vidDrawStopwatch(void);
static void App_vidDrawCountdown(void);


/* ================= Timer1 Callback ================= */

/*
 * Called Every 10 ms.
 * Every 100 Calls Represent One Second.
 */
static void App_vidTimer1Callback(void)
{
    /* Advance ringtone timing with the same 10 ms hardware tick. */
    Buzzer_Service_vidTimerTick10ms();

    App_u8TenMsCounter++;

    if(App_u8TenMsCounter >= APP_TIMER1_MATCHES_PER_SECOND)
    {
        App_u8TenMsCounter = 0;

        if(App_u8SecondsPending < APP_MAX_PENDING_SECONDS)
        {
            App_u8SecondsPending++;
        }
    }
}


/* ================= Application Initialization ================= */

void APP_vidInit(void)
{
    /* Initialize TWI Before Accessing DS1307 */
    TWI_vidInitMaster();

    /* Initialize RTC */
    DS1307_vidInit();

    /* Initialize User Interface Drivers */
    LCD_vidInit();
    KPD_vidInit();

    /* Initialize Shared Buzzer Manager Before The Sound Services */
    Buzzer_Service_vidInit();

    /* Initialize Services */
    Alarm_vidInit();
    Stopwatch_vidInit();
    Countdown_vidInit();

    /* Initialize RTC Cache */
    App_vidRefreshRTC();

    /* Reset Application Timer Variables */
    App_u8TenMsCounter = 0;
    App_u8SecondsPending = 0;

    /* Start On Clock Screen */
    App_eCurrentPage = APP_PAGE_CLOCK;
    App_u8MenuSelection = 0;
    App_u8ClockSetupSelection = 0;
    App_u8Clock24HourMode = 1;
    App_u8RingtoneDraft = BUZZER_RINGTONE_NOKIA;
    App_u8RingtoneSource = BUZZER_SERVICE_SOURCE_ALARM;
    App_u8LCDDirty = 1;

    /* Configure Timer1 Compare Match */
    TMR1_vidSetCompareMatchValue(
        TMR1_CHANNEL_A,
        APP_TIMER1_COMPARE_MATCH_VALUE
    );

    /* Register Callback Before Enabling The Timer Interrupt */
    TMR_vidSetCallback(
        TMR1_INT_OCA,
        App_vidTimer1Callback
    );

    /* Initialize Timer1 */
    TMR1_vidInit();

    /* Enable Timer1 Compare Match A Interrupt */
    TMR1_vidIntEnable(TMR1_INT_OCA);

    /* Enable Global Interrupt */
    GIE_vidEnable();
}


/* ================= Application Task ================= */

/*
 * Call Continuously From main().
 * Periodic Service Tasks Are Executed Once Per Second.
 */
void APP_vidTask(void)
{
    u8 Local_u8RunSecondTasks = 0;
    u8 Local_u8Key;

    /* Read/Update The Shared Pending-Seconds Counter Atomically */
    GIE_vidDisable();

    if(App_u8SecondsPending > 0)
    {
        App_u8SecondsPending--;
        Local_u8RunSecondTasks = 1;
    }

    GIE_vidEnable();

    if(Local_u8RunSecondTasks == 1)
    {
        /* Update Periodic Services */
        Alarm_vidTask();
        Stopwatch_vidTask();
        Countdown_vidTask();

        /* Refresh RTC Cache */
        App_vidRefreshRTC();

        /* Refresh Dynamic Pages */
        if((App_eCurrentPage == APP_PAGE_CLOCK) ||
           (App_eCurrentPage == APP_PAGE_ALARM) ||
           (App_eCurrentPage == APP_PAGE_STOPWATCH) ||
           (App_eCurrentPage == APP_PAGE_COUNTDOWN))
        {
            App_u8LCDDirty = 1;
        }
    }

    /* Read Keypad Event */
    Local_u8Key = KPD_u8GetPressedKey();

    if(Local_u8Key != KPD_NO_PRESSED_KEY)
    {
        App_vidHandleKey(Local_u8Key);
        App_u8LCDDirty = 1;
    }

    /* Update LCD Only When Required */
    if(App_u8LCDDirty == 1)
    {
        App_vidRenderScreen();
        App_u8LCDDirty = 0;
    }

    /* Ringtones advance continuously even when their service page is closed. */
    Buzzer_Service_vidTask();
}


/* ================= RTC Helpers ================= */

static void App_vidRefreshRTC(void)
{
    u8 Local_u8Status;

    Local_u8Status = DS1307_u8GetTime(
        &App_TimeDate.DS1307_Hours,
        &App_TimeDate.DS1307_Minutes,
        &App_TimeDate.DS1307_Seconds
    );

    App_u8TimeValid = (Local_u8Status == DS1307_NO_ERROR);

    Local_u8Status = DS1307_u8GetDate(
        &App_TimeDate.DS1307_Day,
        &App_TimeDate.DS1307_Date,
        &App_TimeDate.DS1307_Month,
        &App_TimeDate.DS1307_Year
    );

    App_u8DateValid = (Local_u8Status == DS1307_NO_ERROR);
}


/*
 * Calculate Day Of Week For Years 2000 -> 2099.
 * Return Value: Sunday = 1, Monday = 2, ..., Saturday = 7.
 */
static u8 App_u8GetDayOfWeek(u8 Copy_u8Date,
                              u8 Copy_u8Month,
                              u8 Copy_u8Year)
{
    u16 Local_u16Year = 2000U + Copy_u8Year;
    u8 Local_u8Month = Copy_u8Month;
    u16 Local_u16ZellerValue;

    if(Local_u8Month < 3)
    {
        Local_u8Month += 12;
        Local_u16Year--;
    }

    Local_u16ZellerValue =
        (u16)Copy_u8Date +
        (u16)((13U * (Local_u8Month + 1U)) / 5U) +
        Local_u16Year +
        (Local_u16Year / 4U) -
        (Local_u16Year / 100U) +
        (Local_u16Year / 400U);

    Local_u16ZellerValue %= 7U;

    /* Zeller: 0=Saturday, 1=Sunday, ..., 6=Friday */
    if(Local_u16ZellerValue == 0)
    {
        return 7;
    }

    return (u8)Local_u16ZellerValue;
}


/* ================= Input Helpers ================= */

static void App_vidStartInput(APP_Page_t Copy_ePage,
                              APP_Page_t Copy_eReturnPage,
                              u8 Copy_u8TargetLength)
{
    u8 Local_u8Index;

    App_eCurrentPage = Copy_ePage;
    App_eInputReturnPage = Copy_eReturnPage;
    App_u8InputLength = 0;
    App_u8InputTargetLength = Copy_u8TargetLength;
    App_u8InputError = 0;

    for(Local_u8Index = 0; Local_u8Index < sizeof(App_au8InputBuffer); Local_u8Index++)
    {
        App_au8InputBuffer[Local_u8Index] = '\0';
    }
}


static u8 App_u8GetInputDigit(u8 Copy_u8Index)
{
    if(Copy_u8Index < App_u8InputLength)
    {
        return App_au8InputBuffer[Copy_u8Index];
    }

    return (u8)'_';
}


static u16 App_u16ParseInput(u8 Copy_u8StartIndex, u8 Copy_u8Length)
{
    u8 Local_u8Index;
    u16 Local_u16Value = 0;

    for(Local_u8Index = 0; Local_u8Index < Copy_u8Length; Local_u8Index++)
    {
        Local_u16Value = (u16)(Local_u16Value * 10U);
        Local_u16Value += (u16)(App_au8InputBuffer[Copy_u8StartIndex + Local_u8Index] - '0');
    }

    return Local_u16Value;
}


static void App_vidCommitInput(void)
{
    u8 Local_u8Status;
    u8 Local_u8Hours;
    u8 Local_u8Minutes;
    u8 Local_u8Seconds;
    u8 Local_u8Date;
    u8 Local_u8Month;
    u8 Local_u8Year;
    u8 Local_u8Day;
    u16 Local_u16Hours;

    if(App_u8InputLength != App_u8InputTargetLength)
    {
        App_u8InputError = 1;
        return;
    }

    switch(App_eCurrentPage)
    {
        case APP_PAGE_SET_TIME:

            Local_u8Hours = (u8)App_u16ParseInput(0, 2);
            Local_u8Minutes = (u8)App_u16ParseInput(2, 2);
            Local_u8Seconds = (u8)App_u16ParseInput(4, 2);

            Local_u8Status = DS1307_u8SetTime(
                Local_u8Hours,
                Local_u8Minutes,
                Local_u8Seconds
            );

            if(Local_u8Status == DS1307_NO_ERROR)
            {
                App_vidRefreshRTC();
                App_eCurrentPage = APP_PAGE_CLOCK_SETUP;
                App_u8InputError = 0;
            }
            else
            {
                App_u8InputError = 1;
            }

            break;

        case APP_PAGE_SET_DATE:

            Local_u8Date = (u8)App_u16ParseInput(0, 2);
            Local_u8Month = (u8)App_u16ParseInput(2, 2);
            Local_u8Year = (u8)App_u16ParseInput(4, 2);

            Local_u8Day = App_u8GetDayOfWeek(
                Local_u8Date,
                Local_u8Month,
                Local_u8Year
            );

            Local_u8Status = DS1307_u8SetDate(
                Local_u8Day,
                Local_u8Date,
                Local_u8Month,
                Local_u8Year
            );

            if(Local_u8Status == DS1307_NO_ERROR)
            {
                App_vidRefreshRTC();
                App_eCurrentPage = APP_PAGE_CLOCK_SETUP;
                App_u8InputError = 0;
            }
            else
            {
                App_u8InputError = 1;
            }

            break;

        case APP_PAGE_SET_ALARM:

            Local_u8Date = (u8)App_u16ParseInput(0, 2);
            Local_u8Month = (u8)App_u16ParseInput(2, 2);
            Local_u8Year = (u8)App_u16ParseInput(4, 2);
            Local_u8Hours = (u8)App_u16ParseInput(6, 2);
            Local_u8Minutes = (u8)App_u16ParseInput(8, 2);

            Local_u8Status = Alarm_u8SetDateTime(
                Local_u8Hours,
                Local_u8Minutes,
                Local_u8Date,
                Local_u8Month,
                Local_u8Year
            );

            if(Local_u8Status == ALARM_NO_ERROR)
            {
                App_eCurrentPage = APP_PAGE_ALARM;
                App_u8InputError = 0;
            }
            else
            {
                App_u8InputError = 1;
            }

            break;

        case APP_PAGE_SET_COUNTDOWN:

            Local_u16Hours = App_u16ParseInput(0, 3);
            Local_u8Minutes = (u8)App_u16ParseInput(3, 2);
            Local_u8Seconds = (u8)App_u16ParseInput(5, 2);

            if(Local_u16Hours > 255U)
            {
                App_u8InputError = 1;
                break;
            }

            Local_u8Status = Countdown_u8SetTime(
                (u8)Local_u16Hours,
                Local_u8Minutes,
                Local_u8Seconds
            );

            if(Local_u8Status == COUNTDOWN_NO_ERROR)
            {
                App_eCurrentPage = APP_PAGE_COUNTDOWN;
                App_u8InputError = 0;
            }
            else
            {
                App_u8InputError = 1;
            }

            break;

        default:
            App_u8InputError = 1;
            break;
    }
}


/* ================= Key Handling ================= */

static void App_vidHandleKey(u8 Copy_u8Key)
{
    /* Numeric Input Pages */
    if((App_eCurrentPage == APP_PAGE_SET_TIME) ||
       (App_eCurrentPage == APP_PAGE_SET_DATE) ||
       (App_eCurrentPage == APP_PAGE_SET_ALARM) ||
       (App_eCurrentPage == APP_PAGE_SET_COUNTDOWN))
    {
        App_u8InputError = 0;

        if((Copy_u8Key >= (u8)'0') && (Copy_u8Key <= (u8)'9'))
        {
            if(App_u8InputLength < App_u8InputTargetLength)
            {
                App_au8InputBuffer[App_u8InputLength] = Copy_u8Key;
                App_u8InputLength++;
            }
        }
        else if(Copy_u8Key == KPD_KEY_STAR)
        {
            if(App_u8InputLength > 0)
            {
                App_u8InputLength--;
                App_au8InputBuffer[App_u8InputLength] = '\0';
            }
        }
        else if(Copy_u8Key == KPD_KEY_HASH)
        {
            App_vidCommitInput();
        }
        else if(Copy_u8Key == KPD_KEY_C)
        {
            App_eCurrentPage = App_eInputReturnPage;
            App_u8InputError = 0;
        }

        return;
    }

    /* Global Mute Shortcut: Silence Any Active Alarm/Countdown Request */
    if(Copy_u8Key == KPD_KEY_HASH)
    {
        Alarm_vidStopRinging();
        Countdown_vidStopRinging();
        return;
    }

    switch(App_eCurrentPage)
    {
        case APP_PAGE_CLOCK:

            if(Copy_u8Key == KPD_KEY_A)
            {
                App_eCurrentPage = APP_PAGE_MENU;
            }

            break;

        case APP_PAGE_MENU:

            if(Copy_u8Key == KPD_KEY_A)
            {
                App_u8MenuSelection = (App_u8MenuSelection + 6U) % 7U;
            }
            else if(Copy_u8Key == KPD_KEY_B)
            {
                App_u8MenuSelection = (App_u8MenuSelection + 1U) % 7U;
            }
            else if(Copy_u8Key == KPD_KEY_C)
            {
                App_eCurrentPage = APP_PAGE_CLOCK;
            }
            else if(Copy_u8Key == KPD_KEY_D)
            {
                switch(App_u8MenuSelection)
                {
                    case 0:
                        App_eCurrentPage = APP_PAGE_CLOCK_SETUP;
                        break;

                    case 1:
                        App_eCurrentPage = APP_PAGE_ALARM;
                        break;

                    case 2:
                        App_eCurrentPage = APP_PAGE_STOPWATCH;
                        break;

                    case 3:
                        App_eCurrentPage = APP_PAGE_COUNTDOWN;
                        break;

                    case 4:
                        App_eCurrentPage = APP_PAGE_CLOCK_FORMAT;
                        break;

                    case 5:
                        App_u8RingtoneSource = BUZZER_SERVICE_SOURCE_ALARM;
                        App_u8RingtoneDraft = Buzzer_Service_u8GetRingtone(
                            BUZZER_SERVICE_SOURCE_ALARM
                        );
                        App_eCurrentPage = APP_PAGE_ALARM_TONE;
                        break;

                    case 6:
                        App_u8RingtoneSource = BUZZER_SERVICE_SOURCE_COUNTDOWN;
                        App_u8RingtoneDraft = Buzzer_Service_u8GetRingtone(
                            BUZZER_SERVICE_SOURCE_COUNTDOWN
                        );
                        App_eCurrentPage = APP_PAGE_COUNTDOWN_TONE;
                        break;

                    default:
                        break;
                }
            }

            break;

        case APP_PAGE_CLOCK_SETUP:

            if((Copy_u8Key == KPD_KEY_A) || (Copy_u8Key == KPD_KEY_B))
            {
                App_u8ClockSetupSelection = (u8)!App_u8ClockSetupSelection;
            }
            else if(Copy_u8Key == KPD_KEY_D)
            {
                if(App_u8ClockSetupSelection == 0)
                {
                    App_vidStartInput(APP_PAGE_SET_TIME,
                                      APP_PAGE_CLOCK_SETUP,
                                      6);
                }
                else
                {
                    App_vidStartInput(APP_PAGE_SET_DATE,
                                      APP_PAGE_CLOCK_SETUP,
                                      6);
                }
            }
            else if(Copy_u8Key == KPD_KEY_C)
            {
                App_eCurrentPage = APP_PAGE_MENU;
            }

            break;

        case APP_PAGE_CLOCK_FORMAT:

            if((Copy_u8Key == KPD_KEY_A) ||
               (Copy_u8Key == KPD_KEY_B) ||
               (Copy_u8Key == KPD_KEY_D))
            {
                App_u8Clock24HourMode = (u8)!App_u8Clock24HourMode;
            }
            else if(Copy_u8Key == KPD_KEY_C)
            {
                App_eCurrentPage = APP_PAGE_MENU;
            }

            break;

        case APP_PAGE_ALARM_TONE:
        case APP_PAGE_COUNTDOWN_TONE:

            if((Copy_u8Key == KPD_KEY_A) || (Copy_u8Key == KPD_KEY_B))
            {
                if(Copy_u8Key == KPD_KEY_A)
                {
                    App_u8RingtoneDraft = (u8)(
                        (App_u8RingtoneDraft + BUZZER_RINGTONE_COUNT - 1U) %
                        BUZZER_RINGTONE_COUNT
                    );
                }
                else
                {
                    App_u8RingtoneDraft = (u8)(
                        (App_u8RingtoneDraft + 1U) % BUZZER_RINGTONE_COUNT
                    );
                }
            }
            else if(Copy_u8Key == KPD_KEY_D)
            {
                Buzzer_Service_u8SetRingtone(
                    App_u8RingtoneSource,
                    App_u8RingtoneDraft
                );

                App_eCurrentPage = APP_PAGE_MENU;
            }
            else if(Copy_u8Key == KPD_KEY_C)
            {
                App_eCurrentPage = APP_PAGE_MENU;
            }

            break;

        case APP_PAGE_ALARM:

            if(Copy_u8Key == KPD_KEY_A)
            {
                App_vidStartInput(APP_PAGE_SET_ALARM,
                                  APP_PAGE_ALARM,
                                  10);
            }
            else if(Copy_u8Key == KPD_KEY_D)
            {
                /* D always toggles the armed state; completed/expired alarms
                 * must be configured again instead of being re-armed. */
                if(Alarm_u8IsEnabled() == 1)
                {
                    Alarm_vidDisable();
                }
                else if((Alarm_u8IsTriggered() == 0) &&
                        (Alarm_u8IsExpired() == 0))
                {
                    Alarm_vidEnable();
                }
            }
            else if(Copy_u8Key == KPD_KEY_HASH)
            {
                Alarm_vidStopRinging();
            }
            else if(Copy_u8Key == KPD_KEY_C)
            {
                App_eCurrentPage = APP_PAGE_MENU;
            }

            break;

        case APP_PAGE_STOPWATCH:

            if(Copy_u8Key == KPD_KEY_D)
            {
                if(Stopwatch_u8IsRunning() == 1)
                {
                    Stopwatch_vidStop();
                }
                else
                {
                    Stopwatch_vidStart();
                }
            }
            else if(Copy_u8Key == KPD_KEY_B)
            {
                Stopwatch_vidReset();
            }
            else if(Copy_u8Key == KPD_KEY_C)
            {
                App_eCurrentPage = APP_PAGE_MENU;
            }

            break;

        case APP_PAGE_COUNTDOWN:

            if(Copy_u8Key == KPD_KEY_A)
            {
                App_vidStartInput(APP_PAGE_SET_COUNTDOWN,
                                  APP_PAGE_COUNTDOWN,
                                  7);
            }
            else if(Copy_u8Key == KPD_KEY_B)
            {
                Countdown_vidReset();
            }
            else if(Copy_u8Key == KPD_KEY_D)
            {
                if(Countdown_u8IsRunning() == 1)
                {
                    Countdown_vidPause();
                }
                else
                {
                    Countdown_vidStart();
                }
            }
            else if(Copy_u8Key == KPD_KEY_HASH)
            {
                Countdown_vidStopRinging();
            }
            else if(Copy_u8Key == KPD_KEY_C)
            {
                App_eCurrentPage = APP_PAGE_MENU;
            }

            break;

        default:
            break;
    }
}


/* ================= LCD Formatting Helpers ================= */

static void App_vidWriteText(const char *Copy_pcText)
{
    u8 Local_u8Index = 0;

    while(Copy_pcText[Local_u8Index] != '\0')
    {
        LCD_vidSendData((u8)Copy_pcText[Local_u8Index]);
        Local_u8Index++;
    }
}


static void App_vidWriteTwoDigits(u8 Copy_u8Value)
{
    LCD_vidSendData((u8)('0' + ((Copy_u8Value / 10U) % 10U)));
    LCD_vidSendData((u8)('0' + (Copy_u8Value % 10U)));
}


static void App_vidWriteThreeDigits(u8 Copy_u8Value)
{
    LCD_vidSendData((u8)('0' + ((Copy_u8Value / 100U) % 10U)));
    LCD_vidSendData((u8)('0' + ((Copy_u8Value / 10U) % 10U)));
    LCD_vidSendData((u8)('0' + (Copy_u8Value % 10U)));
}


static void App_vidWriteFiveDigits(u16 Copy_u16Value)
{
    LCD_vidSendData((u8)('0' + ((Copy_u16Value / 10000U) % 10U)));
    LCD_vidSendData((u8)('0' + ((Copy_u16Value / 1000U) % 10U)));
    LCD_vidSendData((u8)('0' + ((Copy_u16Value / 100U) % 10U)));
    LCD_vidSendData((u8)('0' + ((Copy_u16Value / 10U) % 10U)));
    LCD_vidSendData((u8)('0' + (Copy_u16Value % 10U)));
}


/* Write The Hour According To The Selected Display Format.
 * RTC And Alarm Inputs Remain In 24-hour Format.
 */
static void App_vidWriteDisplayHour(u8 Copy_u8Hours)
{
    u8 Local_u8Hours = Copy_u8Hours;

    if(App_u8Clock24HourMode == 0)
    {
        Local_u8Hours = (u8)(Copy_u8Hours % 12U);

        if(Local_u8Hours == 0)
        {
            Local_u8Hours = 12;
        }
    }

    App_vidWriteTwoDigits(Local_u8Hours);
}


/* Write AM/PM Only In 12-hour Display Mode. */
static void App_vidWriteAmPm(u8 Copy_u8Hours)
{
    if(App_u8Clock24HourMode == 0)
    {
        App_vidWriteText((Copy_u8Hours >= 12U) ? " PM" : " AM");
    }
}


/* ================= LCD Page Drawing ================= */

static void App_vidDrawClock(void)
{
    LCD_vidSendCommand(LCD_CLEAR_DISPLAY);

    LCD_SetCursorPosition(0, 0);
    App_vidWriteText("Time ");

    if(App_u8TimeValid == 1)
    {
        App_vidWriteDisplayHour(App_TimeDate.DS1307_Hours);
        LCD_vidSendData(':');
        App_vidWriteTwoDigits(App_TimeDate.DS1307_Minutes);
        LCD_vidSendData(':');
        App_vidWriteTwoDigits(App_TimeDate.DS1307_Seconds);
        App_vidWriteAmPm(App_TimeDate.DS1307_Hours);
    }
    else
    {
        App_vidWriteText("--:--:--");
    }

    LCD_SetCursorPosition(1, 0);

    if(App_u8DateValid == 1)
    {
        App_vidWriteTwoDigits(App_TimeDate.DS1307_Date);
        LCD_vidSendData('/');
        App_vidWriteTwoDigits(App_TimeDate.DS1307_Month);
        LCD_vidSendData('/');
        App_vidWriteTwoDigits(App_TimeDate.DS1307_Year);
        App_vidWriteText(" A=Menu");
    }
    else
    {
        App_vidWriteText("No date  A=Menu");
    }
}


static void App_vidDrawMenu(void)
{
    LCD_vidSendCommand(LCD_CLEAR_DISPLAY);

    LCD_SetCursorPosition(0, 0);
    App_vidWriteText("Select Mode A/B");

    LCD_SetCursorPosition(1, 0);
    LCD_vidSendData('>');

    switch(App_u8MenuSelection)
    {
        case 0:
            App_vidWriteText("Clock Setup");
            break;

        case 1:
            App_vidWriteText("Alarm");
            break;

        case 2:
            App_vidWriteText("Stopwatch");
            break;

        case 3:
            App_vidWriteText("Countdown");
            break;

        case 4:
            App_vidWriteText("Time Format");
            break;

        case 5:
            App_vidWriteText("Alarm Ringtone");
            break;

        case 6:
            App_vidWriteText("CD Ringtone");
            break;

        default:
            App_vidWriteText("Clock Setup");
            break;
    }
}


static void App_vidDrawClockSetup(void)
{
    LCD_vidSendCommand(LCD_CLEAR_DISPLAY);

    LCD_SetCursorPosition(0, 0);
    App_vidWriteText("Clock Setup");

    LCD_SetCursorPosition(1, 0);
    LCD_vidSendData('>');

    if(App_u8ClockSetupSelection == 0)
    {
        App_vidWriteText("Set Time");
    }
    else
    {
        App_vidWriteText("Set Date");
    }
}


/* Clock display preference changes immediately and applies at runtime. */
static void App_vidDrawClockFormat(void)
{
    LCD_vidSendCommand(LCD_CLEAR_DISPLAY);

    LCD_SetCursorPosition(0, 0);
    App_vidWriteText("Time Format");

    LCD_SetCursorPosition(1, 0);
    App_vidWriteText((App_u8Clock24HourMode == 1) ? "Mode: 24H A/B" : "Mode: 12H A/B");
}


/* Ringtone selection is independent for Alarm and Countdown. */
static void App_vidDrawRingtone(void)
{
    LCD_vidSendCommand(LCD_CLEAR_DISPLAY);

    LCD_SetCursorPosition(0, 0);

    if(App_eCurrentPage == APP_PAGE_ALARM_TONE)
    {
        App_vidWriteText("Alarm Ringtone");
    }
    else
    {
        App_vidWriteText("CD Ringtone");
    }

    LCD_SetCursorPosition(1, 0);

    switch(App_u8RingtoneDraft)
    {
        case BUZZER_RINGTONE_NOKIA:
            App_vidWriteText("Nokia Tune A/B");
            break;

        case BUZZER_RINGTONE_JINGLE_BELLS:
            App_vidWriteText("Jingle Bells A/B");
            break;

        case BUZZER_RINGTONE_WESTMINSTER:
            App_vidWriteText("Westminster A/B");
            break;

        default:
            App_vidWriteText("Nokia Tune A/B");
            break;
    }
}


static void App_vidDrawInput(void)
{
    u8 Local_u8Index;

    LCD_vidSendCommand(LCD_CLEAR_DISPLAY);

    if(App_u8InputError == 1)
    {
        LCD_SetCursorPosition(0, 0);
        App_vidWriteText("Invalid Input");

        LCD_SetCursorPosition(1, 0);
        App_vidWriteText("*=Edit C=Back");
        return;
    }

    LCD_SetCursorPosition(0, 0);

    switch(App_eCurrentPage)
    {
        case APP_PAGE_SET_TIME:
            App_vidWriteText("Set Time 24H");
            break;

        case APP_PAGE_SET_DATE:
            App_vidWriteText("Set Date");
            break;

        case APP_PAGE_SET_ALARM:
            App_vidWriteText("Alarm DDMMYYHHMM");
            break;

        case APP_PAGE_SET_COUNTDOWN:
            App_vidWriteText("Set Countdown");
            break;

        default:
            break;
    }

    LCD_SetCursorPosition(1, 0);

    if(App_eCurrentPage == APP_PAGE_SET_TIME)
    {
        LCD_vidSendData(App_u8GetInputDigit(0));
        LCD_vidSendData(App_u8GetInputDigit(1));
        LCD_vidSendData(':');
        LCD_vidSendData(App_u8GetInputDigit(2));
        LCD_vidSendData(App_u8GetInputDigit(3));
        LCD_vidSendData(':');
        LCD_vidSendData(App_u8GetInputDigit(4));
        LCD_vidSendData(App_u8GetInputDigit(5));
        LCD_vidSendData(' ');
        App_vidWriteText("#=Save");
    }
    else if(App_eCurrentPage == APP_PAGE_SET_DATE)
    {
        LCD_vidSendData(App_u8GetInputDigit(0));
        LCD_vidSendData(App_u8GetInputDigit(1));
        LCD_vidSendData('/');
        LCD_vidSendData(App_u8GetInputDigit(2));
        LCD_vidSendData(App_u8GetInputDigit(3));
        LCD_vidSendData('/');
        LCD_vidSendData(App_u8GetInputDigit(4));
        LCD_vidSendData(App_u8GetInputDigit(5));
        LCD_vidSendData(' ');
        App_vidWriteText("#=Save");
    }
    else if(App_eCurrentPage == APP_PAGE_SET_ALARM)
    {
        LCD_vidSendData(App_u8GetInputDigit(0));
        LCD_vidSendData(App_u8GetInputDigit(1));
        LCD_vidSendData('/');
        LCD_vidSendData(App_u8GetInputDigit(2));
        LCD_vidSendData(App_u8GetInputDigit(3));
        LCD_vidSendData('/');
        LCD_vidSendData(App_u8GetInputDigit(4));
        LCD_vidSendData(App_u8GetInputDigit(5));
        LCD_vidSendData(' ');
        LCD_vidSendData(App_u8GetInputDigit(6));
        LCD_vidSendData(App_u8GetInputDigit(7));
        LCD_vidSendData(':');
        LCD_vidSendData(App_u8GetInputDigit(8));
        LCD_vidSendData(App_u8GetInputDigit(9));
    }
    else if(App_eCurrentPage == APP_PAGE_SET_COUNTDOWN)
    {
        for(Local_u8Index = 0; Local_u8Index < 3; Local_u8Index++)
        {
            LCD_vidSendData(App_u8GetInputDigit(Local_u8Index));
        }

        LCD_vidSendData(':');
        LCD_vidSendData(App_u8GetInputDigit(3));
        LCD_vidSendData(App_u8GetInputDigit(4));
        LCD_vidSendData(':');
        LCD_vidSendData(App_u8GetInputDigit(5));
        LCD_vidSendData(App_u8GetInputDigit(6));
        LCD_vidSendData(' ');
        App_vidWriteText("#=Save");
    }
}


static void App_vidDrawAlarm(void)
{
    u8 Local_u8Hours = 0;
    u8 Local_u8Minutes = 0;
    u8 Local_u8Date = 0;
    u8 Local_u8Month = 0;
    u8 Local_u8Year = 0;

    LCD_vidSendCommand(LCD_CLEAR_DISPLAY);
    LCD_SetCursorPosition(0, 0);

    App_vidWriteText("Alarm ");

    if(Alarm_u8IsConfigured() == 1)
    {
        Alarm_u8GetDateTime(
            &Local_u8Hours,
            &Local_u8Minutes,
            &Local_u8Date,
            &Local_u8Month,
            &Local_u8Year
        );

        App_vidWriteDisplayHour(Local_u8Hours);
        LCD_vidSendData(':');
        App_vidWriteTwoDigits(Local_u8Minutes);
        App_vidWriteAmPm(Local_u8Hours);

        LCD_SetCursorPosition(1, 0);
        App_vidWriteTwoDigits(Local_u8Date);
        LCD_vidSendData('/');
        App_vidWriteTwoDigits(Local_u8Month);
        LCD_vidSendData('/');
        App_vidWriteTwoDigits(Local_u8Year);
        LCD_vidSendData(' ');

        if(Alarm_u8IsRinging() == 1)
        {
            App_vidWriteText("RING");
        }
        else if(Alarm_u8IsExpired() == 1)
        {
            App_vidWriteText("MISSED");
        }
        else if(Alarm_u8IsTriggered() == 1)
        {
            App_vidWriteText("DONE");
        }
        else if(Alarm_u8IsEnabled() == 1)
        {
            App_vidWriteText("ON");
        }
        else
        {
            App_vidWriteText("OFF");
        }
    }
    else
    {
        App_vidWriteText("--:--");
        LCD_SetCursorPosition(1, 0);
        App_vidWriteText("A=Set D=On C=Bk");
    }
}

static void App_vidDrawStopwatch(void)
{
    u16 Local_u16Minutes = 0;
    u8 Local_u8Seconds = 0;

    Stopwatch_u8GetTime(&Local_u16Minutes, &Local_u8Seconds);

    LCD_vidSendCommand(LCD_CLEAR_DISPLAY);
    LCD_SetCursorPosition(0, 0);

    if(Stopwatch_u8IsRunning() == 1)
    {
        App_vidWriteText("Stopwatch RUN");
    }
    else
    {
        App_vidWriteText("Stopwatch PAUSE");
    }

    LCD_SetCursorPosition(1, 0);
    App_vidWriteFiveDigits(Local_u16Minutes);
    LCD_vidSendData(':');
    App_vidWriteTwoDigits(Local_u8Seconds);
    LCD_SetCursorPosition(1, 9);
    App_vidWriteText("B=Reset");
}


static void App_vidDrawCountdown(void)
{
    u8 Local_u8Hours = 0;
    u8 Local_u8Minutes = 0;
    u8 Local_u8Seconds = 0;

    LCD_vidSendCommand(LCD_CLEAR_DISPLAY);

    if(Countdown_u8IsConfigured() == 0)
    {
        LCD_SetCursorPosition(0, 0);
        App_vidWriteText("CD Not Set");

        LCD_SetCursorPosition(1, 0);
        App_vidWriteText("A=Set C=Back");
        return;
    }

    Countdown_u8GetRemainingTime(
        &Local_u8Hours,
        &Local_u8Minutes,
        &Local_u8Seconds
    );

    LCD_SetCursorPosition(0, 0);

    if(Countdown_u8IsFinished() == 1)
    {
        App_vidWriteText("CD DONE #=Mute");
    }
    else if(Countdown_u8IsRunning() == 1)
    {
        App_vidWriteText("CD RUN D=Pause");
    }
    else
    {
        App_vidWriteText("CD PAUSE D=Go");
    }

    LCD_SetCursorPosition(1, 0);
    App_vidWriteThreeDigits(Local_u8Hours);
    LCD_vidSendData(':');
    App_vidWriteTwoDigits(Local_u8Minutes);
    LCD_vidSendData(':');
    App_vidWriteTwoDigits(Local_u8Seconds);
}


static void App_vidRenderScreen(void)
{
    switch(App_eCurrentPage)
    {
        case APP_PAGE_CLOCK:
            App_vidDrawClock();
            break;

        case APP_PAGE_MENU:
            App_vidDrawMenu();
            break;

        case APP_PAGE_CLOCK_SETUP:
            App_vidDrawClockSetup();
            break;

        case APP_PAGE_CLOCK_FORMAT:
            App_vidDrawClockFormat();
            break;

        case APP_PAGE_ALARM_TONE:
        case APP_PAGE_COUNTDOWN_TONE:
            App_vidDrawRingtone();
            break;

        case APP_PAGE_SET_TIME:
        case APP_PAGE_SET_DATE:
        case APP_PAGE_SET_ALARM:
        case APP_PAGE_SET_COUNTDOWN:
            App_vidDrawInput();
            break;

        case APP_PAGE_ALARM:
            App_vidDrawAlarm();
            break;

        case APP_PAGE_STOPWATCH:
            App_vidDrawStopwatch();
            break;

        case APP_PAGE_COUNTDOWN:
            App_vidDrawCountdown();
            break;

        default:
            App_eCurrentPage = APP_PAGE_CLOCK;
            App_vidDrawClock();
            break;
    }
}
