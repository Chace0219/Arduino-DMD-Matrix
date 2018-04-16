/*


    Produced By Colin Weber
    Date: 2016.09

 */
#include "FBD.h"
#include "SimpleTimer.h"
#include <SPI.h>
#include <DMD2.h>

#include <fonts/Arial_black_16.h>
#include <fonts/Arial14.h>
#include <fonts/SystemFont5x7.h>

#define WIDTH = 1;
#define HEIGHT = 1;

//SoftDMD dmd(2,1);  // DMD controls the entire display

SoftDMD dmd(2,1);  // DMD controls the entire display
DMD_TextBox box(dmd, 0, 2);  // "box" provides a text box to automatically write to/scroll the display

SimpleTimer timer; // the timer object

/* Port Definition */
// Button Keys
#define STARTCOUNT 2 // momentary digital input to trigger the count
#define STOPCOUNT 3 // momentary digital input to stop the count (freeze)
#define SCROLLKEY 4 // between scrolling and fixed text 1= scrolling 0= fixed

// LED outputs 
#define LEDSTEP45_15 A0 // high when the countdown is between -45 to -15 minutes
#define LEDSTEP15_5 A1 // high when countdown has reached between  -15 to -5 minutes
#define LEDSTEP5 A2 // high when -5 minutes is reached and or counting up

/* Status Definition */
#define IDLE_STATIC   0 // Static text status
#define IDLE_SCROLL   1 // Scroll text status
#define COUNTUP       2 // Count up Mode
#define COUNTDOWN     3 // Count down Mode
#define COUNTOVER     4 // Count over 1 hour 
#define FREEZE        5 // Count freeze status

// Device status structure variable                                            
struct
{
    unsigned nState : 3; // it is possible 8 status 
    unsigned : 3; //
    unsigned OverFlash: 1; // when Count over, Screen flash indicates
    unsigned Colon : 1; // when clock display, Colon indicates
    unsigned short CurrMin; // Current Minute
    unsigned short CurrSec; // Current Second

    unsigned int nFreezeCnt; // when freeze status, 
}mystatus;

/* Function declaration*/
void ShowClock(unsigned short nMinute, unsigned short nSecond, unsigned short bColonOn);
void LogicControl();
void ShowFlash(unsigned short bFlash);

/* Function block variables */
static TP FlashLEDStep1, FlashLEDStep2; // 
static RTtrg FlashLEDStepIN; // 
static FTtrg FlashLEDStep1OUT;

static TON StartClick, StopClick, ScrollClick, LongStopClick; // Long time Click FBD
static TON SecondTimer; 
static RTtrg StartTrg, StopTrg, ScrollTrg, LongStopTrg;  

#define KEYTIME       4 // 200ms
#define LONGKEYTIME   100 // 5ms
#define SECTIMER  20 // 1sec 

void InitVars()
{
    //
    FlashLEDStep1.IN = 0;
    FlashLEDStep1.PRE = 0;
    FlashLEDStep1.Q = 0;
    FlashLEDStep1.PT = SECTIMER;
    FlashLEDStep1.ET = 0;
    
    FlashLEDStep2.IN = 0;
    FlashLEDStep2.PRE = 0;
    FlashLEDStep2.Q = 0;
    FlashLEDStep2.PT = SECTIMER;
    FlashLEDStep2.ET = 0;

    FlashLEDStepIN.IN = 0;
    FlashLEDStepIN.PRE = 0;
    FlashLEDStepIN.Q = 0;

    FlashLEDStep1OUT.IN = 0;
    FlashLEDStep1OUT.Q = 0;
    FlashLEDStep1OUT.PRE = 0;
    

    SecondTimer.IN = 0;
    SecondTimer.Q = 0;
    SecondTimer.PT = SECTIMER;
    SecondTimer.ET = 0;

    StartClick.IN = 0;
    StartClick.Q = 0;
    StartClick.PT = KEYTIME;
    StartClick.ET = 0;

    StopClick.IN = 0;
    StopClick.Q = 0;
    StopClick.PT = KEYTIME;
    StopClick.ET = 0;
    
    ScrollClick.IN = 0;
    ScrollClick.Q = 0;
    ScrollClick.PT = KEYTIME;
    ScrollClick.ET = 0;

    LongStopClick.IN = 0;
    LongStopClick.Q = 0;
    LongStopClick.PT = LONGKEYTIME;
    LongStopClick.ET = 0;

    StartTrg.IN = 0;
    StartTrg.PRE = 0;
    StartTrg.Q = 0;
    
    StopTrg.IN = 0;
    StopTrg.PRE = 0;
    StopTrg.Q = 0;

    ScrollTrg.IN = 0;
    ScrollTrg.PRE = 0;
    ScrollTrg.Q = 0;

    LongStopTrg.IN = 0;
    LongStopTrg.PRE = 0;
    LongStopTrg.Q = 0;  

    mystatus.nState = IDLE_STATIC;
}

// the setup routine runs once when you press reset:
void setup() 
{
    // Set brightness 
    dmd.setBrightness(50);
  
    // Select font, clear/init the DMD pixels held in RAM
    dmd.clearScreen();   //true is normal (all pixels off), false is negative (all pixels on)
    dmd.selectFont(Arial_Black_16);
    //dmd.beginNoTimer();
 
    dmd.begin();
    
    /* Init Ports */
    // Inputs
    pinMode(STARTCOUNT, INPUT);
    pinMode(STOPCOUNT, INPUT);
    pinMode(SCROLLKEY, INPUT);

    // Outputs
    pinMode(LEDSTEP45_15, OUTPUT);
    pinMode(LEDSTEP15_5, OUTPUT);
    pinMode(LEDSTEP5, OUTPUT);

    InitVars();

    // 
    Serial.begin(9600);
    Serial.println("Start Program!!!");
    // Timer interrupt setting
    timer.setInterval(50, LogicControl);

    //
    mystatus.CurrMin = 45;
    mystatus.CurrSec = 0;
    
}



// the loop routine runs over and over again forever:
void loop() {
    timer.run();
}

/***************************************************
 *  Name:        LogicControl
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  Nothing.
 *  
 *  Description: Show 4 clock numerals on the screen, and select whether the flashing colon is on or off
 *
 ***************************************************/
void LogicControl()
{
    // Serial.println(F("LOGIC"));
    FlashLEDStepIN.IN = ((mystatus.nState == COUNTUP) || (mystatus.nState == COUNTDOWN && mystatus.CurrMin < 5));
    FlashLEDStepIN.IN = FlashLEDStepIN.IN && (FlashLEDStep2.Q == 0);
    RTrgFBD(&FlashLEDStepIN);

    // 
    FlashLEDStep1.IN = FlashLEDStepIN.Q;
    TPFBD(&FlashLEDStep1);
    digitalWrite(LEDSTEP5, FlashLEDStep1.Q);
    FlashLEDStep1OUT.IN = FlashLEDStep1.Q;
    FTrgFBD(&FlashLEDStep1OUT);
    FlashLEDStep2.IN = FlashLEDStep1OUT.Q;
    TPFBD(&FlashLEDStep2);
    
    if((mystatus.nState == COUNTDOWN) && (mystatus.CurrMin >= 5 && mystatus.CurrMin < 15))
        digitalWrite(LEDSTEP15_5, HIGH);
    else
        digitalWrite(LEDSTEP15_5, LOW);

    if((mystatus.nState == COUNTDOWN) && (mystatus.CurrMin >= 15 && mystatus.CurrMin < 45))
    {
        digitalWrite(LEDSTEP45_15, HIGH);
    }
    else
    {
        digitalWrite(LEDSTEP45_15, LOW);
    }

    StartClick.IN = digitalRead(STARTCOUNT);
    TONFBD(&StartClick);
    StartTrg.IN = StartClick.Q;
    RTrgFBD(&StartTrg);
    if(StartTrg.Q)
    {
        Serial.println(F("START"));

        if(mystatus.nState < COUNTUP)
        {
            mystatus.nState = COUNTDOWN;
            mystatus.CurrMin = 45;
            mystatus.CurrSec = 0;
            mystatus.Colon = 1;
        }
    }

    StopClick.IN = digitalRead(STOPCOUNT);
    TONFBD(&StopClick);
    StopTrg.IN = StopClick.Q;
    RTrgFBD(&StopTrg);
    if(StopTrg.Q)
    {
        Serial.println(F("STOP"));
        if(mystatus.nState == COUNTOVER)
        {
            mystatus.nState = IDLE_STATIC;    
        }
        else if(mystatus.nState == COUNTDOWN || mystatus.nState == COUNTUP)
        {
            mystatus.nState = FREEZE;
            mystatus.nFreezeCnt = 0;    
        }
    }

    LongStopClick.IN = digitalRead(STOPCOUNT);
    TONFBD(&LongStopClick);
    LongStopTrg.IN = LongStopClick.Q;
    RTrgFBD(&LongStopTrg);
    if(LongStopTrg.Q)
    {
        Serial.println(F("LongSTOP"));
        if(mystatus.nState == FREEZE)
        {
            Serial.println(F("IDLE"));
            mystatus.nState = IDLE_STATIC;    
        }
    }

    ScrollClick.IN = digitalRead(SCROLLKEY);
    TONFBD(&ScrollClick);
    ScrollTrg.IN = ScrollClick.Q;
    RTrgFBD(&ScrollTrg);
    if(ScrollTrg.Q)
    {
        Serial.println(F("Scroll"));
        if(mystatus.nState == IDLE_STATIC)
            mystatus.nState = IDLE_SCROLL; 
        else if(mystatus.nState == IDLE_SCROLL)
            mystatus.nState = IDLE_STATIC; 
    }
    
    // 
    switch(mystatus.nState)
    {
        case IDLE_SCROLL:
        {
            Serial.print("Scroll");
            // it can be changed.            
            dmd.marqueeScrollY(1);
        }
    }

    SecondTimer.IN = (SecondTimer.Q == 0);
    TONFBD(&SecondTimer);
    if(SecondTimer.Q)
    { // operates per 1 sec

        mystatus.Colon = ~mystatus.Colon;
        switch(mystatus.nState)
        {
            case IDLE_STATIC:
            {
                // static display
                Serial.print("Static");
                dmd.selectFont(SystemFont5x7); // You can change font
                // it can be changed.
                dmd.drawString(0, 0, "");
                dmd.drawString(2, 9, "");
                            
            }
            break;

            case COUNTDOWN:
            {
                Serial.print("Count Down");
                if(mystatus.CurrMin == 0 && mystatus.CurrSec == 0)
                    mystatus.nState = COUNTUP;
                else
                {
                    if(mystatus.CurrSec == 0)
                    {
                        mystatus.CurrSec = 59;
                        mystatus.CurrMin--;
                    }
                    else
                        mystatus.CurrSec--;                    
                    ShowClock(mystatus.CurrMin, mystatus.CurrSec, mystatus.Colon);
                }
            }
            break;    

            case COUNTUP:
            {
                Serial.print("Count Up");
                if(mystatus.CurrMin == 59 && mystatus.CurrSec == 59)
                {
                    mystatus.nState = COUNTOVER;
                    mystatus.OverFlash = false;
                }
                else
                {
                    if(mystatus.CurrSec == 59)
                    {
                        mystatus.CurrSec = 0;
                        mystatus.CurrMin++;
                    }
                    else
                        mystatus.CurrSec++;                    
                    ShowClock(mystatus.CurrMin, mystatus.CurrSec, mystatus.Colon);
                }
            }
            break;

            case COUNTOVER:
            {
                Serial.print("Count Over");
                ShowFlash(mystatus.OverFlash);
                mystatus.OverFlash = ~mystatus.OverFlash;
            }
            break;

            case FREEZE:
            {
                Serial.print("Freeze");
                if(mystatus.nFreezeCnt > 599)
                    mystatus.nState = IDLE_STATIC;
                else
                    mystatus.nFreezeCnt++;
                ShowClock(mystatus.CurrMin, mystatus.CurrSec, true);
            }
            break;

        }
        Serial.print(mystatus.CurrMin);
        Serial.print(",");
        Serial.println(mystatus.CurrSec);
    }
}

/***************************************************
 *  Name:        ShowFlash
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  bFlash: Clear screen or not
 *  Description: When Count over, flash text display
 *
 ***************************************************/
void ShowFlash(unsigned short bFlash)
{
    dmd.selectFont(Arial_Black_16);
    if(bFlash)
        dmd.clearScreen();            
    else
        dmd.drawString(1, 3, F("+1hr"));   // high number of minute
}

/***************************************************
 *  Name:        ShowClock
 *
 *  Returns:     Nothing.
 *
 *  Parameters:  nMinute: Minute numerial, nSecond: Second numerial, 
 *               bColonOn: the flashing colon is on or off
 *  Description: Show 4 clock numerals on the screen, and select whether the flashing colon is on or off
 *
 ***************************************************/
void ShowClock(unsigned short nMinute, unsigned short nSecond, unsigned short bColonOn)
{
    box.print('T');
    box.print('-');
    box.print(nMinute / 10);
    box.print(nMinute % 10);
    if(bColonOn)
        box.println(':');
    else
        box.println(' ');
    
    box.print(nSecond / 10);
    box.print(nSecond % 10);
    box.println(F("s"));
}
