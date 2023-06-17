;
;       base allowable definitions for f14 mission generation
;                                              AS OF 4 AUG 93 (WARGO)
;
;

[worlds]                ; changes to this section must be matched by chages
			;       to code

	NorthCape
	Africa
	Training

[sides]

	friendly
	neutral
	enemy
	civilian
	unknown

[weathers]

	HighSky
	HighClouds
	Overcast


[loadouts]
	fd_bravo
	mc_bravo
	fd_alpha
	fd_charlie
	t_bravo
	mc_charlie
	mc_alpha        syn=t_support
	t_alpha


[object_names]

group = An-22
	An-22a          side=enemy
        An-22b          side=enemy

group = An-26
	An-26a          side=enemy
        An-26b          side=enemy

group = none
	Be-12a          side=enemy
        F-5a            side=enemy

group = F-16
	F-16a           side=friendly
	F-16b           side=neutral
        F-16c           side=friendly

group = none
	G-4a            side=enemy
	Il-38a          side=enemy
        J-22a           side=enemy

group = MiG-21
	MiG-21a         side=enemy
	MiG-21b         side=enemy
	MiG-21c         side=enemy
        MiG-21d         side=enemy

group = MiG-23
	MiG-23a         side=enemy
	MiG-23b         side=enemy
	MiG-23c         side=enemy
        MiG-23d         side=enemy

group = MiG-25
	MiG-25a         side=enemy
	MiG-25b         side=enemy
        MiG-25c         side=enemy

group = MiG-27
	MiG-27a         side=enemy
	MiG-27b         side=enemy
        MiG-27c         side=enemy

group = Mig-29
	MiG-29a         side=enemy
	MiG-29b         side=enemy
	MiG-29c         side=enemy
	MiG-29d         side=enemy
	MiG-29e         side=enemy
        MiG-29f         side=enemy

group = MiG-31
	MiG-31a         side=enemy
	MiG-31b         side=enemy
        MiG-31c         side=enemy

group = F-1
	F-1a            side=enemy
        F-1b            side=enemy

group = Su-17
	Su-17a          side=enemy
	Su-17b          side=enemy
        Su-17c          side=enemy

group = Su-22
	Su-22a          side=enemy
	Su-22b          side=enemy
	Su-22c          side=enemy
        Su-22d          side=enemy

group = Su-24
	Su-24a          side=enemy
	Su-24b          side=enemy
	Su-24c          side=enemy
	Su-24d          side=enemy
        Su-24e          side=enemy

group = Su-25
	Su-25a          side=enemy
	Su-25b          side=enemy
	Su-25c          side=enemy
	Su-25d          side=enemy
        Su-25e          side=enemy

group = Su-27
	Su-27a          side=enemy
	Su-27b          side=enemy
        Su-27c          side=enemy

group = Tu-16
	Tu-16Aa         side=enemy
	Tu-16Da         side=enemy
	Tu-16Ga         side=enemy
	Tu-16Gb         side=enemy
	Tu-16Ha         side=enemy
	Tu-16Ja         side=enemy
        Tu-16La         side=enemy

group = Tu-22
	Tu-22a          side=enemy
	Tu-22b          side=enemy
	Tu-22c          side=enemy
        Tu-22Ma         side=enemy

group = Tu-95
	Tu-95Da         side=enemy
        Tu-95Ga         side=enemy

group = none
	Tu-126a         side=enemy
	Tu-142a         side=enemy
	Viggena         side=neutral
        Viggenb         side=neutral


group = Yak-38
	Yak-38a         side=enemy
	Yak-38b         side=enemy
        Yak-38c         side=enemy

group = A-6
	A-6Ea           side=friendly
	A-6Eb           side=friendly
	A-6Ec           side=friendly
        A-6Ed           side=friendly

group = A-7
	A-7Ea           side=friendly
	A-7Eb           side=friendly
        A-7Ec           side=friendly

group = B-52
	B-52a           side=friendly
	B-52b           side=friendly
        B-52c           side=friendly

group = none
	C-130a          side=friendly
	E-2Ca           side=friendly
        EA-6Ba          side=friendly

group = F/A-18
	F/A-18a         side=friendly
	F/A-18b         side=friendly
	F/A-18c         side=friendly
        F/A-18d         side=friendly

group = F-111
	F-111a          side=friendly
        F-111b          side=friendly

group = GR.1
	GR.1a           side=friendly
        GR.1b           side=friendly

group = none
	KA-6a           side=friendly
	MK.3a           side=friendly
	Nimroda         side=friendly
	P-3a            side=friendly
        S-3a            side=friendly

group = Ka-25
	Ka-25a          side=enemy
        Ka-25b          side=enemy

group = Ka-27
	Ka-27a          side=enemy
        Ka-27b          side=enemy

group = Mi-8
	Mi-8a           side=enemy
        Mi-8b           side=enemy

group = none
	Mi-14a          side=enemy
        Mi-24a          side=enemy

group = SH-2
	SH-2a           side=friendly
        SH-2b           side=friendly

group = SH-3
	SH-3a           side=friendly
        SH-3a           side=friendly

group = none
        SH-60a          side=friendly

group = MH-53
	MH-53a          side=friendly
        MH-53b          side=friendly

group = carrier
	Nimitza         side=friendly
	KHawka          side=friendly
        Forstala        side=friendly

group = none
	CGa             side=friendly
	DDGa            side=friendly
	FFGa            side=friendly
	LSTa            side=friendly
	SovCVa          side=enemy
	SovCVHa         side=enemy
	SovCGHa         side=enemy
	SovBCGa         side=enemy
	SovCGa          side=enemy
	SovDDGa         side=enemy
	SovFFGa         side=enemy
	SovFFa          side=enemy
	SovPTMa         side=enemy
	SovLSTa         side=enemy
	SovAFSa         side=enemy
	Rafta           side=friendly
        Raftb           side=enemy

group = F-14
	F-14FDA         side=friendly
	F-14FDB         side=friendly
	F-14FDC         side=friendly
	F-14MCA         side=friendly
        F-14MCB         side=friendly

group = none
        as-4            side=enemy
        as-6            side=enemy


; 33 groups

[formations]
	bomber_formation
	parade
	cruise
	combatspread
	ladder
	box
	wall
	cap
	strike

[object_definitions]
	callsign
	elevation       syn=elev
	form
	goal*
	heading
	location
	number
	name
	path
	speed
	side
	type

[path_actions]          ; changes to this section must be matched by changes
			;       to code
; definitions with stars do not require an equals (=######)
; they are self contained descriptions

; definitions with a + require the value right of the equals to be matched
;       against the path list for a path name.  Path name must occure before
;       path with +


	cap*
	formation       syn=form
	waypoint*
        gorillabomb*
	refuel*
	cruise_miss*
	sar*
	sar_cap*
	gorilla_cover*
	waypoint_escortable*
	awacs*
	get_fuel*
	elevation       syn=elev
	speed
	attack*
	hawkeye+
	heading
	form_on*
	launch*
	damage*
	ditch*
	explode*
	trooperdrop*
	land*
	formtrain*
	engagetrain*
        training*
        hover


[types]

	player                  side=friendly
	wingman                 side=friendly
	carrier                 side=unknown
	plane                   side=unknown
	boat                    side=unknown
	hostile                 side=unknown
	damaged                 side=unknown
	nofire                  side=unknown
	ready_5                 side=unknown


[end]
