/* tables.c: store/attack/RNG/etc tables and variables

   Copyright (c) 1989 James E. Wilson, Robert A. Koeneke

   This software may be copied and distributed for educational, research, and
   not for profit purposes provided that this copyright and statement are
   included in all such copies. */

#include "constant.h"
#include "config.h"
#include "types.h"

#ifdef ANGBAND_HOU
/* Operating hours for ANGBAND				-RAK-	*/
/*	 X = Open; . = Closed					*/
char  days[7][29] = { "SUN:XXXXXXXXXXXXXXXXXXXXXXXX",
		    "MON:XXXXXXXX.........XXXXXXX",
		    "TUE:XXXXXXXX.........XXXXXXX",
		    "WED:XXXXXXXX.........XXXXXXX",
		    "THU:XXXXXXXX.........XXXXXXX",
		    "FRI:XXXXXXXX.........XXXXXXX",
		    "SAT:XXXXXXXXXXXXXXXXXXXXXXXX" };
#endif

store_type store[MAX_STORES];

/* Store owners have different characteristics for pricing and haggling*/
/* Note: Store owners should be added in groups, one for each store    */
owner_type owners[MAX_OWNERS] = {
{"Rincewind the Chicken  (Human)      General Store",
	  250,	175,  108,    4, 0, 12},
{"Mauglin the Grumpy     (Dwarf)      Armoury"	    ,
	32000,	200,  112,    4, 5,  5},
{"Arndal Beast-Slayer    (Half-Elf)   Weaponsmith"  ,
	10000,	185,  110,    5, 1,  8},
{"Ludwig the Humble      (Human)      Temple"	    ,
	 3500,	175,  109,    6, 0, 15},
{"Ga-nat the Greedy      (Gnome)      Alchemist"    ,
	12000,	220,  115,    4, 4,  9},
{"Luthien Starshine      (Elf)        Magic Shop"   ,
	32000,	175,  110,    5, 2, 11},
{"Durwin the Shifty      (Human)      Black Market" ,
        32000,	250,  190,    10, 0, 5},
{"Your home"   ,
	    1,    1,    1,    1, 1, 1},
{"Bilbo the Friendly     (Hobbit)     General Store",
	  200,	170,  108,    5, 3, 15},
{"Darg-Low the Grim      (Human)      Armoury"	    ,
	10000,	190,  111,    4, 0,  9},
{"Oglign Dragon-Slayer   (Dwarf)      Weaponsmith"  ,
	32000,	195,  112,    4, 5,  8},
{"Gunnar the Paladin     (Human)      Temple"	    ,
	 5000,	185,  110,    5, 0, 23},
{"Mauser the Chemist     (Half-Elf)   Alchemist"    ,
	10000,	190,  111,    5, 1,  8},
{"Buggerby the Great!    (Gnome)      Magic Shop"   ,
	20000,	215,  113,    6, 4, 10},
{"Histor the Goblin      (Orc)        Black Market"   ,
	32000,	250,  190,    10, 6, 5},
{"Your sweet abode"   ,
	    1,    1,    1,    1, 1, 1},
{"Lyar-el the Comely     (Elf)        General Store",
	  300,	165,  107,    6, 2, 18},
{"Decado the Handsome    (Human)      Armoury",
	25000,  200,  112,    4, 5, 10},
{"Ithyl-Mak the Beastly  (Half-Troll) Weaponsmith"  ,
	 3000,	210,  115,    6, 7,  8},
{"Delilah the Pure       (Half-Elf)   Temple"	    ,
	25000,	180,  107,    6, 1, 20},
{"Wizzle the Chaotic     (Hobbit)     Alchemist"    ,
	10000,	190,  110,    6, 3,  8},
{"Inglorian the Mage     (Human?)     Magic Shop"   ,
	32000,	200,  110,    7, 0, 10},
{"Drago the Fair?        (Elf)        Black Market" ,
	32000,	250,  190,    10, 2, 5},
{"Your house"   ,
	    1,    1,    1,    1, 1, 1}
};

/* Buying and selling adjustments for character race VS store	*/
/* owner race							 */
int8u rgold_adj[MAX_RACES][MAX_RACES] = {
			/*Hum, HfE, Elf,  Hal, Gno, Dwa, HfO, HfT, Dun, HiE*/
/*Human		 */	 { 100, 105, 105, 110, 113, 115, 120, 125, 100, 105},
/*Half-Elf	 */	 { 110, 100, 100, 105, 110, 120, 125, 130, 110, 100},
/*Elf		 */	 { 110, 105, 100, 105, 110, 120, 125, 130, 110, 100},
/*Halfling	 */	 { 115, 110, 105,  95, 105, 110, 115, 130, 115, 105},
/*Gnome		 */	 { 115, 115, 110, 105,  95, 110, 115, 130, 115, 110},
/*Dwarf		 */	 { 115, 120, 120, 110, 110,  95, 125, 135, 115, 120},
/*Half-Orc	 */	 { 115, 120, 125, 115, 115, 130, 110, 115, 115, 125},
/*Half-Troll	 */	 { 110, 115, 115, 110, 110, 130, 110, 110, 110, 115},
/*Dunedain 	 */	 { 100, 105, 105, 110, 113, 115, 120, 125, 100, 105},
/*High_Elf	 */	 { 110, 105, 100, 105, 110, 120, 125, 130, 110, 100}
			};

#define MDO MAX_DUNGEON_OBJ

/* object_list[] index of objects that may appear in the store */
int16u store_choice[MAX_STORES][STORE_CHOICES] = {
	/* General Store */
{MDO,MDO,MDO,MDO,MDO,MDO,MDO,MDO,MDO+21,MDO+21,MDO+21,MDO+21,MDO+22,MDO+22,
 MDO+22,MDO+1,MDO+2,MDO+3,MDO+4,
 MDO+22,MDO+20,MDO+21,MDO+5,MDO+6,84,84,123,MDO+22,MDO+22,MDO+21},
	/* Armoury	 */
{103,104,105,106,107,108,109,91,92,125,126,128,129,130,91,92,94,95,96,
 103,104,105,125,128,94,95,111,112,113,121},
	/* Weaponsmith	 */
{29,29,29,31,34,35,42,46,49,58,60,61,63,64,68,73,74,75,77,78,80,82,83,83,
 78,80,82,35,65,66},
	/* Temple	 */
{334,335,336,337,334,335,336,337,257,237,261,262,233,233,240,241,260,
 260,MDO+14,MDO+15,MDO+15,MDO+15,53,54,55,52,335,180,237,240},
	/* Alchemy shop	 */
{227,227,230,230,236,206,252,252,253,253,MDO+7,MDO+7,MDO+7,MDO+8,MDO+8,MDO+8,
 MDO+9,MDO+10,MDO+11,MDO+12,MDO+13,MDO+15,MDO+15,173,174,175,185,185,185,206},
	/* Magic-User store*/
{330,331,332,333,330,331,332,333,326,293,293,299,303,301,302,318,326,
 282,277,279,292,164,167,168,153,137,142,326,328,299}
};

#ifndef MAC
/* MPW doesn't seem to handle this very well, so replace store_buy array
   with a function call on mac */
/* functions defined in sets.c */
extern int general_store(), armory(), weaponsmith(), temple(),
  alchemist(), magic_shop();

int blackmarket();
int home();

/* Each store will buy only certain items, based on TVAL */
int (*store_buy[MAX_STORES])() = {
       general_store, armory, weaponsmith, temple, alchemist, magic_shop,
       blackmarket, home};
#endif

/* Following are arrays for descriptive pieces			*/

#ifdef MACGAME

char **colors;
char **mushrooms;
char **woods;
char **metals;
char **rocks;
char **amulets;
char **syllables;

#else

char *colors[MAX_COLORS] = {
/* Do not move the first three */
  "Icky Green", "Light Brown", "Clear",
  "Azure","Blue","Blue Speckled","Black","Brown","Brown Speckled","Bubbling",
  "Chartreuse","Cloudy","Copper Speckled","Crimson","Cyan","Dark Blue",
  "Dark Green","Dark Red","Gold Speckled","Green","Green Speckled","Grey",
  "Grey Speckled","Hazy","Indigo","Light Blue","Light Green","Magenta",
  "Metallic Blue","Metallic Red","Metallic Green","Metallic Purple","Misty",
  "Orange","Orange Speckled","Pink","Pink Speckled","Puce","Purple",
  "Purple Speckled","Red","Red Speckled","Silver Speckled","Smoky",
  "Tangerine","Violet","Vermilion","White","Yellow", "Purple Speckled",
  "Pungent","Clotted Red","Viscous Pink","Oily Yellow","Gloopy Green",
  "Shimmering","Coagulated Crimson"
};

char *mushrooms[MAX_MUSH] = {
  "Blue","Black","Black Spotted","Brown","Dark Blue","Dark Green","Dark Red",
  "Ecru","Furry","Green","Grey","Light Blue","Light Green","Plaid","Red",
  "Slimy","Tan","White","White Spotted","Wooden","Wrinkled"/*,"Yellow",
  "Shaggy","Red Spotted","Pale Blue","Dark Orange"*/
};

char *woods[MAX_WOODS] = {
  "Aspen","Balsa","Banyan","Birch","Cedar","Cottonwood","Cypress","Dogwood",
  "Elm","Eucalyptus","Hemlock","Hickory","Ironwood","Locust","Mahogany",
  "Maple","Mulberry","Oak","Pine","Redwood","Rosewood","Spruce","Sycamore",
  "Teak","Walnut",
  "Mistletoe","Hawthorn","Bamboo","Silver","Runed","Golden","Ashen"/*,
  "Gnarled","Ivory","Decorative","Willow"*/
};

char *metals[MAX_METALS] = {
  "Aluminum","Cast Iron","Chromium","Copper","Gold","Iron","Magnesium",
  "Molybdenum","Nickel","Rusty","Silver","Steel","Tin","Titanium","Tungsten",
  "Zirconium","Zinc","Aluminum-Plated","Copper-Plated","Gold-Plated",
  "Nickel-Plated","Silver-Plated","Steel-Plated","Tin-Plated","Zinc-Plated",
  "Mithril-Plated","Mithril","Runed","Bronze","Brass","Platinum",
  "Lead"/*,"Lead-Plated","Ivory","Pewter"*/
};

char *rocks[MAX_ROCKS] = {
  "Alexandrite","Amethyst","Aquamarine","Azurite","Beryl","Bloodstone",
  "Calcite","Carnelian","Corundum","Diamond","Emerald","Fluorite","Garnet",
  "Granite","Jade","Jasper","Lapis Lazuli","Malachite","Marble","Moonstone",
  "Onyx","Opal","Pearl","Quartz","Quartzite","Rhodonite","Ruby","Sapphire",
  "Tiger Eye","Topaz","Turquoise","Zircon","Platinum","Bronze",
  "Gold","Obsidian","Silver","Tortoise Shell","Mithril"
};

char *amulets[MAX_AMULETS] = {
  "Amber","Driftwood","Coral","Agate","Ivory","Obsidian",
  "Bone","Brass","Bronze","Pewter","Tortoise Shell","Golden","Azure",
  "Crystal","Silver","Copper"
};

char *syllables[MAX_SYLLABLES] = {
  "a","ab","ag","aks","ala","an","ankh","app",
  "arg","arze","ash","aus","ban","bar","bat","bek",
  "bie","bin","bit","bjor","blu","bot","bu",
  "byt","comp","con","cos","cre","dalf","dan",
  "den","der","doe","dok","eep","el","eng","er","ere","erk",
  "esh","evs","fa","fid","flit","for","fri","fu","gan",
  "gar","glen","gop","gre","ha","he","hyd","i",
  "ing","ion","ip","ish","it","ite","iv","jo",
  "kho","kli","klis","la","lech","man","mar",
  "me","mi","mic","mik","mon","mung","mur","nag","nej",
  "nelg","nep","ner","nes","nis","nih","nin","o",
  "od","ood","org","orn","ox","oxy","pay","pet",
  "ple","plu","po","pot","prok","re","rea","rhov",
  "ri","ro","rog","rok","rol","sa","san","sat",
  "see","sef","seh","shu","ski","sna","sne","snik",
  "sno","so","sol","sri","sta","sun","ta","tab",
  "tem","ther","ti","tox","trol","tue","turs","u",
  "ulk","um","un","uni","ur","val","viv","vly",
  "vom","wah","wed","werg","wex","whon","wun","x",
  "yerg","yp","zun","tri","blaa"
};
#endif

/* used to calculate the number of blows the player gets in combat */
int8u blows_table[11][12] = {
/* STR/W:	   9  18  67 107 117 118  128 138 148 158 168 more  : DEX */
/* <2 */	{  1,  1,  1,  1,  1,  1,   2,  2,  2,  2,  2,   3},
/* <3 */	{  1,  1,  1,  1,  2,  2,   3,  3,  3,  3,  3,   4},
/* <4 */	{  1,  1,  1,  2,  2,  3,   4,  4,  4,  4,  4,   5},
/* <5 */	{  1,  1,  2,  2,  3,  3,   4,  4,  4,  5,  5,   5},
/* <7 */	{  1,  2,  2,  3,  3,  4,   4,  4,  5,  5,  5,   5},
/* <9 */	{  1,  2,  2,  3,  4,  4,   4,  5,  5,  5,  5,   5},
/* <10 */	{  2,  2,  3,  3,  4,  4,   5,  5,  5,  5,  5,   6},
/* <11 */	{  2,  3,  3,  3,  4,  4,   5,  5,  5,  5,  5,   6},
/* <12 */	{  3,  3,  3,  4,  4,  4,   5,  5,  5,  5,  6,   6},
/* <13 */	{  3,  3,  3,  4,  4,  4,   5,  5,  5,  5,  6,   6},
/* >13 */	{  3,  3,  4,  4,  4,  4,   5,  5,  5,  6,  6,   6}
};


/* this table is used to generate a psuedo-normal distribution.	 See the
   function randnor() in misc1.c, this is much faster than calling
   transcendental function to calculate a true normal distribution */
int16u normal_table[NORMAL_TABLE_SIZE] = {
     206,     613,    1022,    1430,	1838,	 2245,	  2652,	   3058,
    3463,    3867,    4271,    4673,	5075,	 5475,	  5874,	   6271,
    6667,    7061,    7454,    7845,	8234,	 8621,	  9006,	   9389,
    9770,   10148,   10524,   10898,   11269,	11638,	 12004,	  12367,
   12727,   13085,   13440,   13792,   14140,	14486,	 14828,	  15168,
   15504,   15836,   16166,   16492,   16814,	17133,	 17449,	  17761,
   18069,   18374,   18675,   18972,   19266,	19556,	 19842,	  20124,
   20403,   20678,   20949,   21216,   21479,	21738,	 21994,	  22245,
   22493,   22737,   22977,   23213,   23446,	23674,	 23899,	  24120,
   24336,   24550,   24759,   24965,   25166,	25365,	 25559,	  25750,
   25937,   26120,   26300,   26476,   26649,	26818,	 26983,	  27146,
   27304,   27460,   27612,   27760,   27906,	28048,	 28187,	  28323,
   28455,   28585,   28711,   28835,   28955,	29073,	 29188,	  29299,
   29409,   29515,   29619,   29720,   29818,	29914,	 30007,	  30098,
   30186,   30272,   30356,   30437,   30516,	30593,	 30668,	  30740,
   30810,   30879,   30945,   31010,   31072,	31133,	 31192,	  31249,
   31304,   31358,   31410,   31460,   31509,	31556,	 31601,	  31646,
   31688,   31730,   31770,   31808,   31846,	31882,	 31917,	  31950,
   31983,   32014,   32044,   32074,   32102,	32129,	 32155,	  32180,
   32205,   32228,   32251,   32273,   32294,	32314,	 32333,	  32352,
   32370,   32387,   32404,   32420,   32435,	32450,	 32464,	  32477,
   32490,   32503,   32515,   32526,   32537,	32548,	 32558,	  32568,
   32577,   32586,   32595,   32603,   32611,	32618,	 32625,	  32632,
   32639,   32645,   32651,   32657,   32662,	32667,	 32672,	  32677,
   32682,   32686,   32690,   32694,   32698,	32702,	 32705,	  32708,
   32711,   32714,   32717,   32720,   32722,	32725,	 32727,	  32729,
   32731,   32733,   32735,   32737,   32739,	32740,	 32742,	  32743,
   32745,   32746,   32747,   32748,   32749,	32750,	 32751,	  32752,
   32753,   32754,   32755,   32756,   32757,	32757,	 32758,	  32758,
   32759,   32760,   32760,   32761,   32761,	32761,	 32762,	  32762,
   32763,   32763,   32763,   32764,   32764,	32764,	 32764,	  32765,
   32765,   32765,   32765,   32766,   32766,	32766,	 32766,	  32766,
};
