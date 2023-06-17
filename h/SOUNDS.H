/*
**            Sound Numbers for F14 Fleet Defender
*/

/* Common sounds */

#define N_AllOff            0 /* All Sounds Off */
#define N_AllFade           1 /* Graceful ending for All Sounds (1-6 on the Stack)*/
#define N_MusicOff          2 /* Immediate ending for Background music */
#define N_MusicFade         3 /* Graceful ending for Background music (1-6 on the Stack) */
#define N_NoiseOff          4 /* Immediate ending for Noise */
#define N_NoiseFade         5 /* Graceful ending for Noise (1-6 on the Stack)*/
#define N_Pause             6 /*  */
#define N_UnPause           7 /*  */
#define N_IsAnySoundOn      8 /* Is Any Sound Still On?? [ Flag in AX ] */

/* Wrapper Numbers */

#define N_ButtonClicks      9 /* Assorted Random Clicks for menu screens */
#define N_PosFeedBack      10 /* Major Menu Feedback sound */
#define N_NegFeedBack      11 /* Menu Feedback sound */
#define N_F14TitleMusic    12 /* Opening Music */
#define N_MenuMus          13 /* Menu Background Music (repeating) */
#define N_MenuCoda         14 /* Ends Menu Music Gracefully */
#define N_GoodEndMus       15 /* "Did Well" Music */
#define N_PoorEndMus       16 /* "Did Poorly" Music */
#define N_DeathMus         17 /* Died in combat */
#define N_PromoMus         18 /* Promoted Music */
#define N_MenuClick        19 /* Minor Menu Feedback sound */
#define N_TextClick        20 /* Text Menu Feedback sound */

#define MaxWrapSnds        20 /* [Same as Last Sound] */

/*    Game Numbers   */

#define N_ButtonClicks      9 /* Assorted Random Clicks for instrument panel */
#define N_FarExplo         10 /* Distant or small Explo */
#define N_LoudExplo        11 /* Distant or small Explo */
#define N_WeGetHitExplo    12 /* Different Explo for us getting hit */
#define N_VulcanCannon     13 /* (Continuous calling = repeated sound)*/
#define N_IFFWarning       14 /* IFF Warning Sound (If Continuous calling -> repeated sound)*/
#define N_AIM9Search       15 /* AIM Searches Sound (Continuous, Turned Off by AIM Lock)*/
#define N_AIM9Lock         16 /* AIM LockOn Sound */
#define N_RadarSearch      17 /* Enemy Radar Searching your area */
#define N_RadarLock        18 /* Enemy Radar Locked on You! */
#define N_SpoolUp          19 /* Sound for any plane Spools Up on carrier deck */
#define N_SpoolDown        20 /* Sound for any plane Spools Down */
#define N_JetPassesBy      21 /* Jet passes by the canopy */
#define N_LoudMissile      22 /* Clunk and Missile Launch */
#define N_StallHorn        23 /* (Continuous calling = repeated sound) */
#define N_DropChaff        24 /* */
#define N_DropBomb         25 /* */
#define N_Eject            26 /* */
#define N_OkJet            27 /* OK to have Jet Sound */
#define N_NoJet            28 /* No Jet Sound now */
#define N_KnotsAre         29 /* Jet Speed on Stack, 0 - 999 */
#define N_JetFromInside    30 /* Jet Sound Heard From Inside Cockpit */
#define N_JetFromOutside   31 /* Jet Sound Heard From Outside Cockpit */
#define N_AirBrakeOn       32 /* AirBrake Starts to move */
#define N_AirBrakeOff      33 /* Stop AirBrake Sound */
#define N_GearOpenSeq      34 /* Doors Open & Gear Moves until stopped by Close */
#define N_GearCloseSeq     35 /* Gear stops & Doors Close */
#define N_CatapultTakeOff  36 /* Sound for any plane making carrier takeoff */
#define N_CarrierLanding   37 /* Landing crunch with tire squeal */
#define N_RedOut           38 /* 0-15 [0=no chg,15=barely audible] vchg for G stress */
#define N_OutgoingJet      39 /* [jet#][0=off,1=start,2=pause,3=cont] */
#define N_IncomingJet      40 /* [jet#][0=off,1=start,2=pause,3=cont] */
#define N_Helicopter       41 /* helicopter - 0 - off, 1 - on */

#define MaxGameSnds        41 /* [Same as Last Sound] */

#define JETOFF              0
#define JETSTART            1
#define JETPAUSE            2
#define JETCONT             3


