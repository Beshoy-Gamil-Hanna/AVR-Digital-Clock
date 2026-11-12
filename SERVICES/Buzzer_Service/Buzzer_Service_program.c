/*
 * Buzzer_Service_program.c
 *
 * Created on: Oct 10, 2026
 * Author: Beshoy
 */

#include <stddef.h>

#include "../../LIB/std_types.h"
#include "../../MCAL/GIE/GIE_interface.h"
#include "../../HAL/BUZZER/BUZZER_interface.h"

#include "Buzzer_Service_interface.h"


/* ================= Private Types ================= */

typedef struct
{
    u16 Buzzer_u16Frequency;
    u8 Buzzer_u8DurationTicks;

} Buzzer_Service_Note_t;


/* ================= Ringtone Data ================= */

/* Durations are in 10 ms units. Zero frequency means silence. */

static const Buzzer_Service_Note_t Buzzer_auNokiaTune[] =
{
    { NOTE_E5, 14 }, { NOTE_D5, 7 },  { NOTE_FS4, 14 }, { NOTE_GS4, 14 },
    { NOTE_CS5, 14 }, { NOTE_B4, 7 }, { NOTE_D4, 14 },  { NOTE_E4, 14 },
    { NOTE_B4, 14 }, { NOTE_A4, 7 },  { NOTE_CS4, 14 }, { NOTE_E4, 14 },
    { NOTE_A4, 24 }, { 0, 24 }
};

static const Buzzer_Service_Note_t Buzzer_auJingleBells[] =
{
    { NOTE_E5, 12 }, { NOTE_E5, 12 }, { NOTE_E5, 24 }, { 0, 5 },
    { NOTE_E5, 12 }, { NOTE_E5, 12 }, { NOTE_E5, 24 }, { 0, 5 },
    { NOTE_E5, 12 }, { NOTE_G5, 12 }, { NOTE_C5, 12 }, { NOTE_D5, 12 },
    { NOTE_E5, 30 }, { 0, 6 },
    { NOTE_F5, 12 }, { NOTE_F5, 12 }, { NOTE_F5, 12 }, { NOTE_F5, 12 },
    { NOTE_F5, 12 }, { NOTE_E5, 12 }, { NOTE_E5, 12 }, { NOTE_E5, 7 },
    { NOTE_E5, 7 }, { NOTE_D5, 12 }, { NOTE_D5, 12 }, { NOTE_E5, 12 },
    { NOTE_D5, 24 }, { NOTE_G5, 24 }, { 0, 25 }
};

/* A short Westminster-style chime phrase. */
static const Buzzer_Service_Note_t Buzzer_auWestminster[] =
{
    { NOTE_G4, 18 }, { NOTE_F4, 18 }, { NOTE_E4, 18 }, { 247, 28 }, { 0, 12 },
    { NOTE_E4, 18 }, { NOTE_G4, 18 }, { NOTE_F4, 18 }, { 247, 28 }, { 0, 12 },
    { 247, 18 }, { NOTE_F4, 18 }, { NOTE_G4, 18 }, { NOTE_E4, 32 }, { 0, 35 }
};


/* ================= Private Variables ================= */

static u8 Buzzer_Service_u8ActiveSources = 0;
static u8 Buzzer_Service_u8AlarmRingtone = BUZZER_RINGTONE_NOKIA;
static u8 Buzzer_Service_u8CountdownRingtone = BUZZER_RINGTONE_JINGLE_BELLS;
static u8 Buzzer_Service_u8CurrentSource = 0;
static u8 Buzzer_Service_u8CurrentRingtone = 0;
static u8 Buzzer_Service_u8NoteIndex = 0;
static u8 Buzzer_Service_u8RemainingTicks = 0;

static volatile u16 Buzzer_Service_u16PendingTicks = 0;


/* ================= Private Function Prototypes ================= */

static u8 Buzzer_Service_u8GetSelectedSource(void);
static u8 Buzzer_Service_u8GetSelectedRingtone(u8 Copy_u8Source);
static const Buzzer_Service_Note_t *Buzzer_Service_pGetMelody(
    u8 Copy_u8Ringtone,
    u8 *Copy_pu8Length
);
static void Buzzer_Service_vidStartSelectedMelody(void);
static void Buzzer_Service_vidLoadCurrentNote(void);
static void Buzzer_Service_vidAdvanceNote(void);


/* ================= Private Functions ================= */

static u8 Buzzer_Service_u8GetSelectedSource(void)
{
    /* Alarm Has Priority If Both Events Are Active. */
    if((Buzzer_Service_u8ActiveSources & BUZZER_SERVICE_SOURCE_ALARM) != 0)
    {
        return BUZZER_SERVICE_SOURCE_ALARM;
    }

    if((Buzzer_Service_u8ActiveSources & BUZZER_SERVICE_SOURCE_COUNTDOWN) != 0)
    {
        return BUZZER_SERVICE_SOURCE_COUNTDOWN;
    }

    return 0;
}


static u8 Buzzer_Service_u8GetSelectedRingtone(u8 Copy_u8Source)
{
    if(Copy_u8Source == BUZZER_SERVICE_SOURCE_ALARM)
    {
        return Buzzer_Service_u8AlarmRingtone;
    }

    if(Copy_u8Source == BUZZER_SERVICE_SOURCE_COUNTDOWN)
    {
        return Buzzer_Service_u8CountdownRingtone;
    }

    return BUZZER_RINGTONE_NOKIA;
}


static const Buzzer_Service_Note_t *Buzzer_Service_pGetMelody(
    u8 Copy_u8Ringtone,
    u8 *Copy_pu8Length
)
{
    const Buzzer_Service_Note_t *Local_pMelody = Buzzer_auNokiaTune;
    u8 Local_u8Length = (u8)(sizeof(Buzzer_auNokiaTune) / sizeof(Buzzer_auNokiaTune[0]));

    switch(Copy_u8Ringtone)
    {
        case BUZZER_RINGTONE_JINGLE_BELLS:
            Local_pMelody = Buzzer_auJingleBells;
            Local_u8Length = (u8)(sizeof(Buzzer_auJingleBells) / sizeof(Buzzer_auJingleBells[0]));
            break;

        case BUZZER_RINGTONE_WESTMINSTER:
            Local_pMelody = Buzzer_auWestminster;
            Local_u8Length = (u8)(sizeof(Buzzer_auWestminster) / sizeof(Buzzer_auWestminster[0]));
            break;

        case BUZZER_RINGTONE_NOKIA:
        default:
            break;
    }

    if(Copy_pu8Length != NULL)
    {
        *Copy_pu8Length = Local_u8Length;
    }

    return Local_pMelody;
}


static void Buzzer_Service_vidLoadCurrentNote(void)
{
    u8 Local_u8Length;
    const Buzzer_Service_Note_t *Local_pMelody;

    Local_pMelody = Buzzer_Service_pGetMelody(
        Buzzer_Service_u8CurrentRingtone,
        &Local_u8Length
    );

    if(Local_u8Length == 0)
    {
        BUZZER_vidStop();
        Buzzer_Service_u8RemainingTicks = 10;
        return;
    }

    if(Buzzer_Service_u8NoteIndex >= Local_u8Length)
    {
        Buzzer_Service_u8NoteIndex = 0;
    }

    Buzzer_Service_u8RemainingTicks =
        Local_pMelody[Buzzer_Service_u8NoteIndex].Buzzer_u8DurationTicks;

    if(Local_pMelody[Buzzer_Service_u8NoteIndex].Buzzer_u16Frequency == 0)
    {
        BUZZER_vidStop();
    }
    else
    {
        BUZZER_vidPlay(
            Local_pMelody[Buzzer_Service_u8NoteIndex].Buzzer_u16Frequency
        );
    }
}


static void Buzzer_Service_vidStartSelectedMelody(void)
{
    Buzzer_Service_u8CurrentSource = Buzzer_Service_u8GetSelectedSource();

    if(Buzzer_Service_u8CurrentSource == 0)
    {
        Buzzer_Service_u8RemainingTicks = 0;
        BUZZER_vidStop();
        return;
    }

    Buzzer_Service_u8CurrentRingtone =
        Buzzer_Service_u8GetSelectedRingtone(Buzzer_Service_u8CurrentSource);

    Buzzer_Service_u8NoteIndex = 0;
    Buzzer_Service_u8RemainingTicks = 0;

    Buzzer_Service_vidLoadCurrentNote();
}


static void Buzzer_Service_vidAdvanceNote(void)
{
    u8 Local_u8Length;

    (void)Buzzer_Service_pGetMelody(
        Buzzer_Service_u8CurrentRingtone,
        &Local_u8Length
    );

    if(Local_u8Length == 0)
    {
        Buzzer_Service_u8NoteIndex = 0;
    }
    else
    {
        Buzzer_Service_u8NoteIndex++;

        if(Buzzer_Service_u8NoteIndex >= Local_u8Length)
        {
            Buzzer_Service_u8NoteIndex = 0;
        }
    }

    Buzzer_Service_vidLoadCurrentNote();
}


/* ================= Public Functions ================= */

void Buzzer_Service_vidInit(void)
{
    Buzzer_Service_u8ActiveSources = 0;
    Buzzer_Service_u8AlarmRingtone = BUZZER_RINGTONE_NOKIA;
    Buzzer_Service_u8CountdownRingtone = BUZZER_RINGTONE_JINGLE_BELLS;
    Buzzer_Service_u8CurrentSource = 0;
    Buzzer_Service_u8CurrentRingtone = 0;
    Buzzer_Service_u8NoteIndex = 0;
    Buzzer_Service_u8RemainingTicks = 0;
    Buzzer_Service_u16PendingTicks = 0;

    BUZZER_vidStop();
}


void Buzzer_Service_vidRequest(u8 Copy_u8Source)
{
    u8 Local_u8PreviousSource;

    if((Copy_u8Source != BUZZER_SERVICE_SOURCE_ALARM) &&
       (Copy_u8Source != BUZZER_SERVICE_SOURCE_COUNTDOWN))
    {
        return;
    }

    Local_u8PreviousSource = Buzzer_Service_u8GetSelectedSource();
    Buzzer_Service_u8ActiveSources |= Copy_u8Source;

    if(Buzzer_Service_u8GetSelectedSource() != Local_u8PreviousSource)
    {
        Buzzer_Service_vidStartSelectedMelody();
    }
}


void Buzzer_Service_vidRelease(u8 Copy_u8Source)
{
    u8 Local_u8PreviousSource;

    if((Copy_u8Source != BUZZER_SERVICE_SOURCE_ALARM) &&
       (Copy_u8Source != BUZZER_SERVICE_SOURCE_COUNTDOWN))
    {
        return;
    }

    Local_u8PreviousSource = Buzzer_Service_u8GetSelectedSource();
    Buzzer_Service_u8ActiveSources &= (u8)(~Copy_u8Source);

    if(Buzzer_Service_u8GetSelectedSource() != Local_u8PreviousSource)
    {
        Buzzer_Service_vidStartSelectedMelody();
    }
}


u8 Buzzer_Service_u8IsActive(u8 Copy_u8Source)
{
    if((Copy_u8Source != BUZZER_SERVICE_SOURCE_ALARM) &&
       (Copy_u8Source != BUZZER_SERVICE_SOURCE_COUNTDOWN))
    {
        return 0;
    }

    return ((Buzzer_Service_u8ActiveSources & Copy_u8Source) != 0);
}


u8 Buzzer_Service_u8SetRingtone(u8 Copy_u8Source, u8 Copy_u8Ringtone)
{
    u8 Local_u8CurrentSource;

    if(Copy_u8Ringtone >= BUZZER_RINGTONE_COUNT)
    {
        return 1;
    }

    if(Copy_u8Source == BUZZER_SERVICE_SOURCE_ALARM)
    {
        Buzzer_Service_u8AlarmRingtone = Copy_u8Ringtone;
    }
    else if(Copy_u8Source == BUZZER_SERVICE_SOURCE_COUNTDOWN)
    {
        Buzzer_Service_u8CountdownRingtone = Copy_u8Ringtone;
    }
    else
    {
        return 1;
    }

    Local_u8CurrentSource = Buzzer_Service_u8GetSelectedSource();

    if(Local_u8CurrentSource == Copy_u8Source)
    {
        Buzzer_Service_vidStartSelectedMelody();
    }

    return 0;
}


u8 Buzzer_Service_u8GetRingtone(u8 Copy_u8Source)
{
    return Buzzer_Service_u8GetSelectedRingtone(Copy_u8Source);
}


/* Called directly from the Timer1 Compare Match ISR every 10 ms. */
void Buzzer_Service_vidTimerTick10ms(void)
{
    if(Buzzer_Service_u16PendingTicks < 65535U)
    {
        Buzzer_Service_u16PendingTicks++;
    }
}


/* Called continuously from the main loop, never from an ISR. */
void Buzzer_Service_vidTask(void)
{
    u16 Local_u16Ticks;
    u8 Local_u8SelectedSource;

    /* Take An Atomic Snapshot Of The 16-bit ISR Counter. */
    GIE_vidDisable();
    Local_u16Ticks = Buzzer_Service_u16PendingTicks;
    Buzzer_Service_u16PendingTicks = 0;
    GIE_vidEnable();

    Local_u8SelectedSource = Buzzer_Service_u8GetSelectedSource();

    if(Local_u8SelectedSource != Buzzer_Service_u8CurrentSource)
    {
        Buzzer_Service_vidStartSelectedMelody();
    }
    else if((Local_u8SelectedSource != 0) &&
            (Buzzer_Service_u8CurrentRingtone !=
             Buzzer_Service_u8GetSelectedRingtone(Local_u8SelectedSource)))
    {
        Buzzer_Service_vidStartSelectedMelody();
    }

    while((Local_u16Ticks > 0) && (Buzzer_Service_u8CurrentSource != 0))
    {
        if(Buzzer_Service_u8RemainingTicks > 0)
        {
            Buzzer_Service_u8RemainingTicks--;
        }

        if(Buzzer_Service_u8RemainingTicks == 0)
        {
            Buzzer_Service_vidAdvanceNote();
        }

        Local_u16Ticks--;
    }
}
