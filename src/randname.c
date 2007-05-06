/*
 * randname.c - random name generation
 * Copyright (c) 2007 Antony Sidwell and others,
 * Based on W. Sheldon Simms name generator originally in randart.c
 *
 * This file is distributed under the terms of both the Angband licence and
 * under the GPL licence (version 2 or any later version).  It may be 
 * redistributed under the terms of either licence. 
 */
#include "angband.h"
#include "randname.h"

/* 
 * Arrays of purely alphabetical, lower-case strings to teach 
 * the name generator with. 
 */
static cptr tolkien_names[] =
{
	"adanedhel", "adurant", "aeglos", "aegnor", "aelin", "aeluin",
	"aerandir", "aerin", "agarwaen", "aglareb", "aglarond", "aglon",
	"ainulindale", "ainur", "alcarinque", "aldaron", "aldudenie",
	"almaren", "alqualonde", "aman", "amandil", "amarie", "amarth",
	"amlach", "amon", "amras", "amrod", "anach", "anar", "anarion",
	"ancalagon", "ancalimon", "anarrima", "andor", "andram", "androth",
	"anduin", "andunie", "anfauglir", "anfauglith", "angainor", "angband",
	"anghabar", "anglachel", "angrenost", "angrim", "angrist", "angrod",
	"anguirel", "annael", "annatar", "annon", "annuminas", "apanonar",
	"aradan", "aragorn", "araman", "aranel", "aranruth", "aranwe", "aras",
	"aratan", "aratar", "arathorn", "arda", "ardgalen", "aredhel",
	"arfeiniel", "argonath", "arien", "armenelos", "arminas", "arnor",
	"aros", "arossiach", "arthad", "arvernien", "arwen", "ascar",
	"astaldo", "atalante", "atanamir", "atanatari", "atani", "aule",
	"avallone", "avari", "avathar", "balan", "balar", "balrog", "barad",
	"baragund", "barahir", "baran", "baranduin", "bar", "bauglir",
	"beleg", "belegaer", "belegost", "belegund", "beleriand", "belfalas",
	"belthil", "belthronding", "beor", "beraid", "bereg", "beren",
	"boromir", "boron", "bragollach", "brandir", "bregolas", "bregor",
	"brethil", "brilthor", "brithiach", "brithombar", "brithon", "cabed",
	"calacirya", "calaquendi", "calenardhon", "calion", "camlost",
	"caragdur", "caranthir", "carcharoth", "cardolan", "carnil",
	"celeborn", "celebrant", "celebrimbor", "celebrindal", "celebros",
	"celegorm", "celon", "cirdan", "cirith", "cirth", "ciryatan",
	"ciryon", "coimas", "corollaire", "crissaegrim", "cuarthal",
	"cuivienen", "culurien", "curufin", "curufinwe", "curunir",
	"cuthalion", "daedeloth", "daeron", "dagnir", "dagor", "dagorlad",
	"dairuin", "danwedh", "delduwath", "denethor", "dimbar", "dimrost",
	"dinen", "dior", "dirnen", "dolmed", "doriath", "dorlas",
	"dorthonion", "draugluin", "drengist", "duath", "duinath", "duilwen",
	"dunedain", "dungortheb", "earendil", "earendur", "earnil", "earnur",
	"earrame", "earwen", "echor", "echoriath", "ecthelion", "edain",
	"edrahil", "eglador", "eglarest", "eglath", "eilinel", "eithel",
	"ekkaia", "elbereth", "eldalie", "eldalieva", "eldamar", "eldar",
	"eledhwen", "elemmire", "elende", "elendil", "elendur", "elenna",
	"elentari", "elenwe", "elerrina", "elleth", "elmoth", "elostirion",
	"elrond", "elros", "elu", "eluchil", "elured", "elurin", "elwe",
	"elwing", "emeldir", "endor", "engrin", "engwar", "eol", "eonwe",
	"ephel", "erchamion", "ereb", "ered", "erech", "eregion", "ereinion",
	"erellont", "eressea", "eriador", "eru", "esgalduin", "este", "estel",
	"estolad", "ethir", "ezellohar", "faelivrin", "falas", "falathar",
	"falathrim", "falmari", "faroth", "fauglith", "feanor", "feanturi",
	"felagund", "finarfin", "finduilas", "fingolfin", "fingon", "finwe",
	"firimar", "formenos", "fornost", "frodo", "fuin", "fuinur",
	"gabilgathol", "galad", "galadriel", "galathilion", "galdor", "galen",
	"galvorn", "gandalf", "gaurhoth", "gelion", "gelmir", "gelydh", "gil",
	"gildor", "giliath", "ginglith", "girith", "glaurung", "glingal",
	"glirhuin", "gloredhel", "glorfindel", "golodhrim", "gondolin",
	"gondor", "gonnhirrim", "gorgoroth", "gorlim", "gorthaur", "gorthol",
	"gothmog", "guilin", "guinar", "guldur", "gundor", "gurthang",
	"gwaith", "gwareth", "gwindor", "hadhodrond", "hador", "haladin",
	"haldad", "haldan", "haldar", "haldir", "haleth", "halmir", "handir",
	"harad", "hareth", "hathaldir", "hathol", "haudh", "helcar",
	"helcaraxe", "helevorn", "helluin", "herumor", "herunumen",
	"hildorien", "himlad", "himring", "hirilorn", "hisilome",
	"hithaeglir", "hithlum", "hollin", "huan", "hunthor", "huor", "hurin",
	"hyarmendacil", "hyarmentir", "iant", "iaur", "ibun", "idril",
	"illuin", "ilmare", "ilmen", "iluvatar", "imlach", "imladris", "indis",
	"ingwe", "irmo", "isil", "isildur", "istari", "ithil", "ivrin",
	"kelvar", "kementari", "ladros", "laiquendi", "lalaith", "lamath",
	"lammoth", "lanthir", "laurelin", "leithian", "legolin", "lembas",
	"lenwe", "linaewen", "lindon", "lindorie", "loeg", "lomelindi",
	"lomin", "lomion", "lorellin", "lorien", "lorindol", "losgar",
	"lothlann", "lothlorien", "luin", "luinil", "lumbar", "luthien",
	"mablung", "maedhros", "maeglin", "maglor", "magor", "mahanaxar",
	"mahtan", "maiar", "malduin", "malinalda", "mandos", "manwe", "mardil",
	"melian", "melkor", "menegroth", "meneldil", "menelmacar",
	"meneltarma", "minas", "minastir", "mindeb", "mindolluin", "mindon",
	"minyatur", "mirdain", "miriel", "mithlond", "mithrandir", "mithrim",
	"mordor", "morgoth", "morgul", "moria", "moriquendi", "mormegil",
	"morwen", "nahar", "naeramarth", "namo", "nandor", "nargothrond",
	"narog", "narsil", "narsilion", "narya", "nauglamir", "naugrim",
	"ndengin", "neithan", "neldoreth", "nenar", "nenning", "nenuial",
	"nenya", "nerdanel", "nessa", "nevrast", "nibin", "nienna", "nienor",
	"nimbrethil", "nimloth", "nimphelos", "nimrais", "nimras",
	"ningloron", "niniel", "ninniach", "ninquelote", "niphredil",
	"nirnaeth", "nivrim", "noegyth", "nogrod", "noldolante", "noldor",
	"numenor", "nurtale", "obel", "ohtar", "oiolosse", "oiomure", "olorin",
	"olvar", "olwe", "ondolinde", "orfalch", "ormal", "orocarni",
	"orodreth", "orodruin", "orome", "oromet", "orthanc", "osgiliath",
	"osse", "ossiriand", "palantir", "pelargir", "pelori", "periannath",
	"quendi", "quenta", "quenya", "radagast", "radhruin", "ragnor",
	"ramdal", "rana", "rathloriel", "rauros", "region", "rerir",
	"rhovanion", "rhudaur", "rhun", "rhunen", "rian", "ringil", "ringwil",
	"romenna", "rudh", "rumil", "saeros", "salmar", "saruman", "sauron",
	"serech", "seregon", "serinde", "shelob", "silmarien", "silmaril",
	"silpion", "sindar", "singollo", "sirion", "soronume", "sul", "sulimo",
	"talath", "taniquetil", "tar", "taras", "tarn", "tathren", "taur",
	"tauron", "teiglin", "telchar", "telemnar", "teleri", "telperion",
	"telumendil", "thalion", "thalos", "thangorodrim", "thargelion",
	"thingol", "thoronath", "thorondor", "thranduil", "thuringwethil",
	"tilion", "tintalle", "tinuviel", "tirion", "tirith", "tol", "tulkas",
	"tumhalad", "tumladen", "tuna", "tuor", "turambar", "turgon", "turin",
	"uial", "uilos", "uinen", "ulairi", "ulmo", "ulumuri", "umanyar",
	"umarth", "umbar", "ungoliant", "urthel", "uruloki", "utumno", "vaire",
	"valacirca", "valandil", "valaquenta", "valar", "valaraukar",
	"valaroma", "valier", "valimar", "valinor", "valinoreva", "valmar",
	"vana", "vanyar", "varda", "vasa", "vilya", "vingilot", "vinyamar",
	"voronwe", "wethrin", "wilwarin", "yavanna",
	NULL
};

/* These are (mostly) picked at random from a Latin word list. */
static cptr scroll_names[] =
{
	"abracadabra", "piffpaffpouf", "izzy", "wizzy", "letsgetsbusy",
	"justlikethat", "hocus", "pocus", "shazam", "please", "abduco",
	"absorbeo", "abutor", "accipio", "acerbus", "adamo", "adeo", "adficio",
	"adinventitias", "admitto", "adsidue", "adsumo", "advenio", "aeger",
	"aegrus", "aestivus", "aggero", "ago", "alioqui", "aliquantum",
	"aliquot", "alter", "ambianis", "amissio", "amoveo", "andegavense",
	"animus", "antepono", "appareo", "appropinquo", "ara", "arca", "arguo",
	"articulus", "ascit", "astrum", "atrebatum", "attonbitus", "audax",
	"aureus", "autus", "averto", "bardus", "bene", "bibo", "bonus",
	"breviter", "calamitas", "callidus", "canonus", "caput", "caritas",
	"casso", "catervatim", "causa", "cedo", "celeriter", "centum",
	"certus", "charisma", "cimentarius", "cito", "clamor", "claustrum",
	"coerceo", "cogo", "colloco", "coma", "cometissa", "commeo",
	"commissum", "commoneo", "compatior", "compes", "compositio",
	"comprovincialis", "concido", "conculco", "condico", "confiteor",
	"conicio", "conor", "conservo", "consisto", "constans", "construo",
	"consueta", "consulto", "consuo", "contamino", "contemptio",
	"contentus", "contineo", "contradictio", "conventus", "copiose",
	"corrigo", "corturiacum", "crastinus", "creo", "cribrum", "cruentus",
	"cubicularius", "cui", "culpa", "cum", "cunctus", "cur", "curiosus",
	"currus", "curto", "custodiae", "debeo", "decimus", "decumbo",
	"defendo", "defleo", "degenero", "delego", "delinquo", "demonstro",
	"dens", "depono", "deprecor", "derelinquo", "desino", "despero",
	"detego", "devito", "diabolus", "didicerat", "differo", "dignitas",
	"dilabor", "diluculo", "diripio", "disco", "dissimilis", "districtus",
	"diutius", "divitiae", "dolens", "domesticus", "donec", "duco",
	"dulcidine", "duro", "ecclesia", "edo", "efficio", "ego", "elemosina",
	"eluvies", "emineo", "emptio", "epistula", "equus", "erogo", "erudio",
	"etsi", "ex", "excito", "excuso", "exesto", "exhorresco", "eximo",
	"exorior", "experior", "expiscor", "explico", "exsequor", "exstinguo",
	"extra", "fabula", "facio", "faenum", "famulus", "fautor", "felix",
	"ferme", "festinus", "fides", "fines", "flamma", "fluo", "formo",
	"fortiter", "foveo", "frequentia", "frugalitas", "fuga", "fultus",
	"furor", "galea", "genus", "glacialis", "gratia", "gravatus",
	"gregatim", "hactenus", "harum", "heniis", "hic", "his", "hoc",
	"honorabilis", "horum", "huic", "humanitas", "hunnam", "iam", "idem",
	"ignavus", "illa", "illarum", "illi", "illis", "illius", "illorum",
	"illum", "imitor", "immortalis", "impedito", "impendo", "imperium",
	"importunus", "ex", "in", "incertus", "includo", "incorruptus", "inda",
	"indignatio", "induco", "inexpugnabilis", "infecunditas", "infenso",
	"infero", "inficio", "infigo", "infirmitas", "infitior", "inflatus",
	"infligo", "infortunatus", "ingemuo", "inicio", "innotesco", "inquis",
	"insciens", "insidiae", "insperatus", "instructus", "insurgo",
	"intentio", "interdum", "intueor", "invado", "invetero", "invisus",
	"ipsemet", "irritus", "ita", "iucunditas", "iugis", "iussu", "jaculum",
	"juventus", "labiosus", "laboriose", "labruscum", "lacertosus",
	"lacrimo", "lacto", "laedo", "laetificus", "laevus", "lamentatio",
	"laqueus", "laudo", "lectus", "lemiscus", "lenitas", "lentesco", "leo",
	"lesciense", "leto", "levidensis", "levo", "liber", "libere", "licet",
	"lino", "loci", "longe", "lucerna", "ludio", "lupus", "macero",
	"maculo", "maero", "magus", "malmundarium", "mandatum", "manus",
	"matera", "me", "mei", "memor", "mensa", "meridianus", "mica",
	"millies", "minuo", "miser", "mitigo", "modio", "molestia",
	"monasteriense", "monstrum", "morior", "moveo", "multo", "mundus",
	"munus", "mutuo", "nascor", "navigo", "necesse", "nemo", "neque",
	"niger", "nisi", "nivellensem", "noceo", "non", "nos", "nota", "novus",
	"nunc", "nusquam", "obdormio", "oblivio", "obviam", "occupo", "odium",
	"omitto", "onus", "oporotheca", "opportunitatus", "ops", "orator",
	"orior", "ostium", "pactum", "palam", "par", "parens", "paro",
	"passer", "patiens", "patruus", "pax", "pecus", "penitus", "per",
	"percutio", "perfectus", "periclitatus", "periurium", "perpetro",
	"perseverantia", "persuadeo", "pertineo", "peruro", "pessimus",
	"pestis", "phasma", "pictoratus", "pirus", "placide", "plagiarius",
	"plecto", "plico", "pluma", "plures", "pluvia", "polenta", "pomum",
	"porta", "postea", "postulo", "potius", "praebeo", "praecido",
	"praeda", "praefinio", "praenuntio", "praesentia", "praesto",
	"praetereo", "pravitas", "premo", "primitus", "principatus", "prius",
	"pro", "procedo", "procurator", "proficio", "profor", "progenero",
	"progressio", "proinde", "prolecto", "proloquor", "prominens",
	"promitto", "promptu", "promus", "propello", "propositum", "prorsus",
	"prosum", "proveho", "prudens", "pudendus", "puerilis", "pugnax",
	"pulex", "pulpa", "pumilius", "punitor", "purgatio", "puteo",
	"pyropus", "quadrigae", "quae", "quaesitio", "qualiscumque", "quam",
	"quando", "quantum", "quantuslibet", "quas", "quater", "quercus",
	"queror", "quibus", "quicquid", "quin", "quisnam", "quo", "quodammodo",
	"quorum", "radicitus", "recedo", "recolo", "rectum", "redarguo",
	"redeo", "refectorium", "rego", "relinquo", "remaneo", "rependo",
	"repeto", "reprehendo", "resisto", "retraho", "revoco", "rigor",
	"rotomagense", "rursus", "sacrilegus", "saeta", "saluto",
	"sanctimonialis", "sanitas", "sapienter", "satis", "scaldus", "scelus",
	"scio", "se", "secundum", "secus", "seductor", "sententia",
	"sepulchrum", "sermo", "servus", "siccus", "silens", "silva",
	"simulatio", "singultus", "sive", "soleo", "sollicitudo", "solum",
	"somnio", "sopor", "speciosus", "spero", "spolium", "statim", "stipes",
	"studio", "sub", "subseco", "succendo", "suggero", "summissus",
	"super", "superne", "suppellex", "suppono", "suscito", "tabellae",
	"tactus", "tam", "tandem", "taruennam", "tempero", "templum", "tendo",
	"tenus", "teres", "tergum", "tero", "tertius", "textor", "thema",
	"thymbra", "titulus", "torqueo", "toties", "traiectum", "trellum",
	"tricesimus", "tristis", "tubineus", "tungris", "turpe", "ubi",
	"ultra", "undique", "unus", "usque", "ut", "utilitas", "utroque",
	"vado", "validus", "vehementer", "vendolius", "ventus", "verecundia",
	"versus", "vesica", "vestio", "vicissitudo", "video", "vilicus",
	"vindico", "virga", "vis", "vivo", "voco", "volo", "voluntas", "volva",
	"voro", "vulariter", "vulnus", "vultus",
	NULL
};

/* Markers for the start and end of words. */
#define S_WORD 26
#define E_WORD S_WORD
#define TOTAL  27

typedef unsigned short name_probs[S_WORD+1][S_WORD+1][TOTAL+1];

/*
 * This function builds probability tables from a list of purely alphabetical
 * lower-case words, and puts them into the supplied name_probs object.
 * The array of names should have a NULL entry at the end of the list.
 * It relies on the ASCII character set (through use of A2I).
 */
static void build_prob(name_probs probs, cptr *learn)
{
	int c_prev, c_cur, c_next;
	cptr ch;
	int i;

	/* Build raw frequencies */
	for (i = 0; learn[i] != NULL; i++)
	{
		c_prev = c_cur = S_WORD;
		ch = learn[i];

		/* Iterate over the next word */
		while (*ch != '\0')
		{
			c_next = A2I(tolower((unsigned char)*ch));

			probs[c_prev][c_cur][c_next]++;
			probs[c_prev][c_cur][TOTAL]++;
                        
			/* Step on */
			c_prev = c_cur;
			c_cur = c_next;
			ch++;
		}

		probs[c_prev][c_cur][E_WORD]++;
		probs[c_prev][c_cur][TOTAL]++;
	}
}

/*
 * Use W. Sheldon Simms' random name generator algorithm (Markov Chain stylee).
 * 
 * Generate a random word using the probability tables we built earlier.  
 * Relies on the A2I and I2A macros (and so the ASCII character set) and 
 * is_a_vowel (so the basic 5 English vowels).
 */
size_t make_word(randname_type name_type, size_t min, size_t max, char *word_buf, size_t buflen)
{
	size_t lnum;
	bool found_word = FALSE;

	static name_probs lprobs;
	static randname_type cached_type = RANDNAME_NUM_TYPES;

	assert(name_type > 0 && name_type < RANDNAME_NUM_TYPES);

	/* To allow for a terminating character */
	assert(buflen > max);

	/* We cache one set of probabilities, only regenerate when
	   the type changes.  It's as good a way as any for now.
	   Frankly, we could probably regenerate every time. */
	if (cached_type != name_type)
	{
		cptr *wordlist = NULL;

		switch (name_type)
		{
			case RANDNAME_TOLKIEN:
			{
				wordlist = tolkien_names;
				break;
			}
			case RANDNAME_SCROLL:
			{
				wordlist = scroll_names;
				break;
			}
		}

		build_prob(lprobs, wordlist);
		cached_type = name_type;
	}
        
	/* Generate the actual word wanted. */
	while (!found_word)
	{
		char *cp = word_buf;
		int c_prev = S_WORD;
		int c_cur = S_WORD;
		int tries = 0;
		bool contains_vowel = FALSE;
		lnum = 0;

		/* We start the word again if we run out of space or have
		   had to have 10 goes to find a word that satisfies the
		   minimal conditions. */
		while (tries < 10 && lnum <= max && !found_word)
		{
			/* Pick the next letter based on a simple weighting
			  of which letters can follow the previous two */
			int r = rand_int(lprobs[c_prev][c_cur][TOTAL]);
			int c_next = 0;

			while (r >= lprobs[c_prev][c_cur][c_next])
			{
				r -= lprobs[c_prev][c_cur][c_next];
				c_next++;
			}
            
			if (c_next == E_WORD)
			{
				/* If we've reached the end, we check if we've
				   met the simple conditions, otherwise have
				   another go at choosing a letter for this
				   position. */
				if (lnum >= min && contains_vowel)
				{
					*cp = '\0';
					found_word = TRUE;
				}
				else
				{
					tries++;
				}
			}
			else
			{
				/* Add the letter to the word and move on. */
				*cp = I2A(c_next);

				if (is_a_vowel(*cp))
					contains_vowel = TRUE;

				cp++;
				lnum++;
				c_prev = c_cur;
				c_cur = c_next;
			}
		}
	}

	return lnum;
}


/* 
 * To run standalone tests, #define RANDNAME_TESTING and link with
 *  with just z-rand.c from Angband. 
 */
#ifdef RANDNAME_TESTING

#include <stdio.h>
#include <time.h>


bool is_a_vowel(int ch)
{
	switch (ch)
	{
		case 'a':
		case 'e':
		case 'i':
		case 'o':
		case 'u':
			 return (TRUE);
	}

	return (FALSE);
}

int main(int argc, char *argv[])
{
	int i;
	char name[256];

	Rand_value = time(NULL);

	for (i = 0; i < 20; i++)
	{
		make_word(RANDNAME_TOLKIEN, 5, 9, name, 256);
		name[0] = toupper((unsigned char) name[0]);
		printf("%s\n", name);
	}

	return 0;
}
#endif
