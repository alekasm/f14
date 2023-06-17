typedef struct {
	long x;
	long z;
	unsigned int heading;
} catch;

typedef struct {
	long x1;
	long z1;
	long x2;
	long z2;
	long cx;
	long cz;
} wire;


typedef struct {
	long x;						/* Center of carrier */
	long y;
	long z;
	unsigned int heading;		/* Carrier Heading */
	int pitch;
	int roll;
	unsigned int wcheading;
	long deckht;				/* Deck height */
	catch catpult[4];
	wire wires[4];
	long cbx1;					/* carrier Bounding box */
	long cbz1;
	long cbx2;
	long cbz2;
	long wbx1;					/* Wire Bounding box */
	long wbx2;
	long wbz1;
	long wbz2;
   long twrx1;					/* tower Bounding box */
	long twrz1;
	long twrx2;
	long twrz2;
} carriers;

