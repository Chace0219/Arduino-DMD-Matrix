


/*
    In order to prevent the shake of Key input signal, I use time delay function block.
    It is called that TON FBD in IEC61131-5 standard.

    Only IN signal is maintained for specified time, Q output become 1.
    Otherwise Q is 0.
*/
typedef struct timeronblock TON;
struct timeronblock
{
  unsigned IN: 1; // IN option
  unsigned Q: 1; // Output
  unsigned PT: 10; // Set point
  unsigned ET: 10; // Elapsed time
};

typedef struct timerPulse TP;
struct timerPulse
{
  unsigned IN: 1; // IN option
  unsigned PRE: 1; // PRE Status
  unsigned Q: 1; // Output
  unsigned PT: 9; // Set point
  unsigned ET: 9; // Elapsed time
};

// FBD process function
void TPFBD(TP *pTP)
{
    if(pTP->IN != pTP->PRE)
    {
        pTP->PRE = pTP->IN;
        if(pTP->PRE)
        {
            pTP->ET = pTP->PT;
            pTP->Q = 1;  
        }
    }

    if(pTP->ET > 0)
        pTP->ET--;
    else
        pTP->Q = 0;
}

// FBD process function
void TONFBD(TON *pTP)
{
  if(pTP->IN)
  {
    if(pTP->ET < pTP->PT)
      pTP->ET++;  
    else
      pTP->Q = 1;  
  }
  else
  {
      pTP->ET = 0;
      pTP->Q = 0;
  }
}

/*
 *  
 *  In order to detect rising edge of input signal, I use Rising trigger function block.
 *  It is called that R_trig FBD in IEC61131-5.
 *  
 *  function: I have used it in order to detect key pressed signal so that I noticed to program that mode is changed.
*/
typedef struct RisingTrg RTtrg;
struct RisingTrg
{
    unsigned IN : 1;
    unsigned PRE : 1;
    unsigned Q : 1;
};

// FBD processing routine
void RTrgFBD(RTtrg *pTrg)
{
    pTrg->Q = 0;
    if(pTrg->IN != pTrg->PRE)
    {
        pTrg->PRE = pTrg->IN;
        if(pTrg->IN)
          pTrg->Q = 1;    
    }
}

typedef struct FallingTrg FTtrg;
struct FallingTrg
{
    unsigned IN : 1;
    unsigned PRE : 1;
    unsigned Q : 1;
};

// FBD processing routine
void FTrgFBD(FTtrg *pTrg)
{
    pTrg->Q = 0;
    if(pTrg->IN != pTrg->PRE)
    {
        pTrg->PRE = pTrg->IN;
        if(pTrg->IN == 0)
            pTrg->Q = 1;    
    }
}

