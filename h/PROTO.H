/*  Planes.c  */
void AvoidMountains ();
void Attacker();
void StraightAttack();
void FriendlyChooseBehavior();
void ChooseBehavior();
void Ladder();
void Ladder2();
void LadderCircle ();
void Box();
void Champagne();
void Wall();
void Wall2();
void FreeFighter();
void FlySanitize();
void CheckGunFiring(COORD tdistl, int tanglel, int tpitchl);
void CoreCheckGunFiring(COORD tdistl, int tanglel, int tpitchl, int turretangle, int fireangle, int dangle, int tdistfire);
void ShowAIStatus(int ainum, int otherai);
void BOMB_TrackingAI();
void DESIGNATED_TrackingAI(int relangle);
void AIBombTarget();
void GetEnemyTarget(int ainum);
void LowBombTarget();
void AIGetBombDist();
void TurnAIPlaneBy(int ainum, int relangle);
void InitAddedWingmen ();
int NearWingTarget(int wingnum);
int AIRadarForwardCheck(int wingnum, int showinfo, int CAPrange);
int AIRadarRearCheck(int wingnum, int showinfo, int Bearrange);
int AICoreRadarCheck(int wingnum, int showinfo, int otherrange, int lessangle, int greaterangle);
void DoVSeperation(int ainum);
void ShowAIWeaponStatus(int ainum);
void GetWingName(int ainum, char *Str);
void AISanitizeCheck(int wingnum);
void AISanitizeGroupCheck(int wingnum, int enemynum, int *planecheck);
void GiveSanitizeMessage(int wingnum, int *groupcheck);
int ProcessAICommMenu(int ainum);
void ChangeGroupBehavior(int ainum, int matchenemyfire);
int IsCapFlier(int ainum);
void AddSomePlanes (int planeid, void  (*AIBehavior)(), int numadd, int numwingadd, int isfriend);
int AddSomePlanes2 (int planenum, int planeid, int AIBehavior, int formnum, int numadd, int numwingadd, int isfriend, int whattype, long xval, long yval, int altval, int headval, int path, int speedval, int howgood);
void UpdateHelicopterHPR_SandP();
void UpdateI(int newi);
int ImNotaFighter(int planenum, int checkguns);
int CheckDetection(int planenum);

/*  Planes2  */
void InitOurWingPlane ();
void SetUpWingman(int leadnum, int b, int numadd, int planeid);
void UpdateWing();
int GetSquadLeader(int planenum);
int GetBehaviorLeader(int planenum);
int GetLeaderCore(int planenum, int nobehave);
void CheckGunFiring(COORD tdistl, int tanglel, int tpitchl);
void CheckAITarget(int ainum);
void FormationSpeedAdjust(int prespeed, int relangle, int tdist, int prewing);
void FormationSpeedAdjustUS(int prespeed, int relangle, int tdist, int prewing);
void Jink();
void Formation();
void FormationUS();
void BracketLeft();
void BracketRight();
void FlyStraight();
void CheckRadarSet(int tdist, int tangle);
void Jink2();
void PostHole();
void BaseBracket(int angleinc, int angleevade);
void DoAICounters(int ainum, int distnm, struct Sams *enemymissile);
void EvadeMissile(int ainum);
void SlewOurWingmen(long xslew, long yslew);
void DamageAI(int planenum);
void GetSortInfo(int ainum);
void GiveSortInfo(int rangarray[], int numarray[], int distarray[], int targetnum);
void CheckCAP();
int CAPAI(int side, int ovalheight, int ovalwidth, int ovallength, int ovaldist);
int FlyTo(int xdim, int ydim, int altdim, int ignoredist);
void SetUpCAP1();
void SetUpCAP2();
void SetUpCAP3();
void CAPTurn1();
void CAPTurn2();
void CAPLeg1();
void CAPLeg2();
void SetUpPrimaryCAP();
int TranslateBehavior(int planenum, int behavenum, int formnum);
void FlyRefuel();
void HelpPlayerRefuel(int pdist, int pangle);
void FlyWayPoint();
void ShowJettison(int planenum);
void HoldSpeedRefuel();
void GetRefuelPlane();
void AIRefuel();
void AIWaitRefuel();
void AILockedRefuel();
void ReleaseRefuel();
void NextRefuel();
int CheckDisengage();
void HelpAIRefuel();
void DoHover();
void GetAIBase(int ainum);
void AIGoHome();
void AIGoHome2();
void CheckAISix(int wingnum);
void GorillaBomb();
int Langle(long x, long y);
void LGetTargADP(int planenum, long dx, long dy, COORD *tdist, int *tpitch, int *tangle, COORD *tdistnm, int *enemyalt, int deltalt, long *ldist);
void LFlyTo(long xdim, long ydim, int altdim, int prewing, int prespeed);
long LDist2D(long x, long y);
long absL(long x);
void DoFormationSpeedAdjust(long prespeed, int relangle, long tdist, int prewing);
long sinXL(int tangle, long idist);
long cosXL(int tangle, long idist);
int SetUpOval1(int ovalheight, int ovalwidth, int ovallength);
int SetUpOval2(int ovalheight, int ovalwidth, int ovallength);
int SetUpOval3(int ovalheight, int ovalwidth, int ovallength);
int OvalTurn1(int ovalheight, int ovalwidth, int ovallength);
int OvalTurn2(int ovalheight, int ovalwidth, int ovallength);
void SetUpRefuel1();
void SetUpRefuel2();
void RefuelTurn1();
void RefuelTurn2();
void RefuelLeg1();
void RefuelLeg2();
void CheckHelpPlayer();
int FirstTargInGroup(int prewing);

/*  planes3.c  */
void InterceptControl(int planenum);
void CheckIntercept(int planenum);
void GetInterceptPlane(int planenum, int prewing);
int CheckIfCanIntercept(int planenum, int tdist);
void UpdateInterceptGroup(int ainum, int dobehave);
int GetCloseAirBase(int ainum, int friend, COORD *distval);
int LaunchPlanesOnGround (int basenum, int isfriend, int targetplane, int behaveval);
void LocationPicture (int whotoldorg, int whatis, int whatangle, COORD whatdist, int planedetect, int nochange);
//  void FlyE2C();
void CheckE2C(int planenum);
void TrooperDrop(int planenum);
void LaunchCruise(int planenum);
void FlyCruise();
void GetLastPosition (int planenum, long *aixloc, long *aiyloc, UWORD *palt);
void FlyWPFollowPlayer();
void FollowPlayer();
void GorillaCover();
void RearCover();
int SARCAP0(int whatangle, long whatdist, int whatalt);
void SARCAP1();
void SARCAP2();
void SARCAP3();
int GetWayPointChanges(int numorgwp, int *behavenum, int *formnum, int *nspeed, int *elevation, int *heading, int *attack, int *launch, int *wpflag);
int DoNextWayPt(int planenum, int waypt);
void GetWayXYZ(int waynum, long *xval, long *yval, UWORD *zval);
void GetPlanePointXYZ(int planenum, long *xval, long *yval, UWORD *zval);
void Ready5();
int ReturnBaseInfo(int basenum, COORD *xloc, COORD *yloc, UWORD *zloc, int *basehead);
void ShowWhichBehave(int planenum);
void ShowWhichBehave2(int planenum);
int GetPlayerHelp();
void ProcessOtherComm();
void DoCAPReplace(int planenum);
void SARWait();
void SAR();
void GetSAR(int planenum);
void GetSARCover(int planenum);
void RestoreCover(int planenum);
int IsBombing(int planenum);
int FriendStatus(int planenum);
int FriendMatch(int plane1, int plane2);
int FriendMatchType(int plane1, int isfriend);
void UpdateOurWPS(int lastwp, int updateall);
int CloseEnoughFor(int whatcheck, int x, int y);
int GetPlaneOrgNum(int planenum);
int CheckForMobileR5(int basenum, int isfriend);
int MovePlane(int planenum, int xval, int yval);
void CruiseDistruct(int planenum);
void CheckPlaneBlast(int xval, int yval, UWORD zval, int blastrad, int iscruise, int camefrom, int missiletype);
void EngageTrain();
void JustPutHere(int planenum, long xval, long yval, UWORD altval, int prewing);
void SetUpLaunch(int planenum, int waypt);
void DoAltHover();
int GetRadarGTNum(int radarnum);
int GetGTRadarNum(int gtnum);
void IndirectWPCorrection(int planenum);
void LetWingmenContinue(int planenum);
int GetDAltSpeed(UWORD alt);
void CheckTurnStuff(int planenum);
void AICarrierLand();
void MoveAIToElevator(int planenum, int elevator);
void DoLandWait(long dx, long dy, int rearhead);
void CheckPlaneLand(int planenum);
void LetNextPlaneLand();
void GetPlaneLanding();
int GetWhichElevator(int try0, int try1, int try2, int try3, int yesinvis);
int CheckIsWayTarget(int bogeyi);
void DoGroupAction(int planenum, int actionflag, int waypt);
void DoOClockCall();
void Immelmann();
void SplitS();

/*  VDI.c  */
int VdiCrt (int CrtNr);
VDITacxy (COORD x, COORD y);

/*  DDD.c  */
DoDDD (int CrtNr);
DDDxy (COORD x, COORD y);

/*  TID.c  */
DoTID (int CrtNr);
TIDxy (COORD x, COORD y);

/*  TAC.c  */
DoTac (int CrtNr);
TacBox (int color);
Tacxy (COORD x,COORD y);

/*  bullets.c  */
int CheckBulletBetter(long x, long y, int z, struct Tracer *t, int targnum);
int IsDeckPlane(int whofired, int targnum);

/*  sams.c  */
int CheckBlastArea(long xval, long yval, int weapon, int cruise);

/*  boats.c  */
void doboats ();
void UpdateBoatHeadPitchRoll (int bnum);
UpdateBoatSpeedAndPosition (int bnum);
int IsBoat(int itemid);
void UpdateOurCarrier(int boatnum);
void UpdateAIsOnCarrier(int boatnum, long cdx, long cdy);
void boatsearching ();
void CheckBoatGunFiring();
void FormationSpeedAdjustBoat(int prespeed, int relangle, int tdist, int prewing);
void AvoidLand();

/* carr.c */
int TurnTo(int planenum, int heading);
int OnTheDeck(long X, long Y, long Z );

/*  aaahit.c  */
void doaaa();
void CheckAAAHit(int planenum, int gtnum);

