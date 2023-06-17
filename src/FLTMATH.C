#include "library.h"
#include "world.h"
#include "sdata.h"
#include "cockpit.h"
#include "f15defs.h"

extern  SWORD   IntDiv();
extern  UWORD   Iasin(),Iacos();

extern  SWORD   FM[3][3];
extern  UWORD   OurHead;
extern  SWORD   OurPitch, OurRoll;

/************************************
*                                   *
*   Multiply matrix A by matrix B   *
*   Store results back into flight  *
*   matrix FM.                      *
*                                   *
************************************/

MatrixMul (A,B)      /* Multiply Matrix A by B-> store into FM */

SWORD   A[3][3],B[3][3];

{
    SWORD TM[3][3];

    MatMul (A,B, TM);   /* Gotta use temp array 'cuz MatMul stores as it goes */

    memcpy (FM, TM, sizeof (FM));

}

/*<f>*/
/**********************************************************
*                                                         *
*   This function will take the 9 elements of the flight  *
*   matrix(FM) and decompose them into a Heading, Pitch,  *
*   and Roll.                                             *
*                                                         *
**********************************************************/

Decompose() {

    SWORD  CosPitch;

    OurPitch = Iasin(-FM[1][2]);  /* Now have Pitch  */
    CosPitch = Icos(OurPitch);

    if (CosPitch != 0) {  /* When Pitch is 90 deg., we have a special case */

        if (abs(FM[0][2])<(int)(23169)) {
            OurHead = Iasin(abs(IntDiv(FM[0][2],CosPitch)));
        }else{
            OurHead = Iacos(abs(IntDiv(FM[2][2],CosPitch)));
        }

      /*  Set for correct quadrant  */
        if (FM[0][2]<=0 && FM[2][2]<0) OurHead += 32768;
        if (FM[0][2]>0 && FM[2][2]<0) OurHead = 16384L + (16384L - OurHead);
        if (FM[0][2]<0 && FM[2][2]>0) OurHead = 49152L + (16384L - OurHead);

        if (abs(FM[1][0])<(int)(23169)) {
            OurRoll = Iasin(abs(IntDiv(FM[1][0],CosPitch)));
        } else{
            OurRoll = Iacos(abs(IntDiv(FM[1][1],CosPitch)));
        }

      /*  Set for correct quadrant  */
        if (FM[1][0]<=0 && FM[1][1]<0) OurRoll += 32768;
        if (FM[1][0]>0  && FM[1][1]<0) OurRoll  = 16384L + (16384L - OurRoll);
        if (FM[1][0]<0  && FM[1][1]>0) OurRoll  = 49152L + (16384L - OurRoll);

    } else {  /* Do this if our Pitch is 90 degrees */
        OurRoll = 0;
        OurHead = Iasin(FM[0][1]);

      /*  Set for correct quadrant  */
        if (FM[1][0]<=0 && FM[1][1]<0) OurHead += 32768;
        if (FM[1][0]>0 && FM[1][1]<0) OurHead = 16384L + (16384L - OurHead);
        if (FM[1][0]<0 && FM[1][1]>0) OurHead = 49152l + (16384L - OurHead);
    }

}


abs (int x) {           // REPLACES REGULAR LIBRARY VERSION !!!!!
    if (x==0x8000) {
        return 0x7fff;
    } else if (x<0) {
        return -x;
    } else {
        return (x);
    }
}

/*<f><s> Integer Math Routines*/

/*******************************************
*                                          *
*   This function performs a division      *
*   of two signed 15 bit fractions.   A/B  *
*                                          *
*******************************************/

SWORD   IntDiv (A,B)

SWORD A,B;
{
    SBYTE   S1 = 1,S2 = 1;
    unsigned long LA,LB;

    if (A < 0)  S1 = -1;

    if (B < 0)  S2 = -1;

    LA = (A<0) ? -A : A;
    LB = (B<0) ? -B : B;

    return ((((LA << 16) / LB) >> 1) * S1 * S2);


}

/*************************************************
*                                                *
*   Function returns a 16 bit Word Degree angle  *
*   of the Arc Sin of A.                         *
*                                                *
*************************************************/


extern int TRGTB[];

unsigned Iasin (A)
int A;
{
    int AA,angl,delta,i;

    if (A==-0x8000) return -0x4000;

    AA = abs(A);
    for (i=(AA>>9)+1; i>=0; i--) {
        if (AA>=TRGTB[i]) {
           delta = TRGTB[i+1] - TRGTB[i];
           angl = ((i)<<8) + (long) 256*(AA-TRGTB[i])/delta;
           break;
        }
    }
    if (A<0) angl = -angl;
    return angl;
}

/*************************************************
*                                                *
*   Function returns a 16 bit Word Degree angle  *
*   of the Arc Cos of A.                         *
*                                                *
*************************************************/

unsigned Iacos (A)
int A;
{
    return 0x4000 - Iasin(A);
}

Sqrt (N)
int N;
{   int A,B;

    N = abs(N);
    if (N<4) return 1;
    B = N>>2;
    do {
        A = N/B;
        B = (B+A)>>1;
    } while (abs(B-A)>1);
    return B;
}

