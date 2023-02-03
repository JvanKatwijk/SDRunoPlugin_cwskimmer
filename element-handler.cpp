
#include	"element-handler.h"
#include	<stdio.h>
//#include	<algorithm>
#include	<cstring>
#include	<vector>
//#include	"radio.h"

#include	"SDRunoPlugin_cwskimmerUi.h"

//#define	QUEUE_LENGTH	32
	elementHandler::elementHandler (SDRunoPlugin_cwskimmerUi *mr,
	                                int ident): 
	                             smoothenSamples (6) {
	this	-> m_form	=  mr;
	this	-> identity	= ident;
	reset	();
}

	elementHandler::~elementHandler () {}

void	elementHandler::reset	() {
	cwCurrent		= 0;
	agc_peak		= 0;
	spaceLevel		= 0;
	avg			= 0;
	starter			= 0;
	cwState			= MODE_IDLE;
	cwText			= std::string ("");
//
//	a PARIS bit is 1200 milliseconds, we use a 2 msec interval
//	so a PARIS bit takes here 600 samples
//	All length are expressed in samples
	currentTime		= 0;
	fillerP			= 0;
	emptyP			= 0;
}

void	elementHandler::process	(float value, int freq, float new_avg) {

//	if (identity != 12)
//	   return;
//
	value = smoothenSamples.filter(value);
	avg	= 0.99 * avg + 0.01 * value;
	if (starter < 100) {
	   spaceLevel = 0.5 * avg;
	   starter ++;
	   return;
	}

	if (value > agc_peak)
	   agc_peak = decayingAverage (agc_peak, value, 25.0);
	else
	   agc_peak = decayingAverage (agc_peak, value, 500.0);

	this	-> cwFrequency	= freq;
//
//	the incoming samplerate is 1000 / 2 samples/second,
//	i.e. app 500 
	currentTime	+= 1;

	switch (cwState) {
	   case MODE_IDLE:
	      if (value > 0.8 * agc_peak) {
	         cwStartTime	= currentTime;
	         cwState	= MODE_TONE;
	         last_peak = value;
	      }
	      break;

	   case MODE_TONE:
	      if (value < 0.7 * last_peak) {	// end of tone?
	          int duration = currentTime - cwStartTime;
	          if (duration < 3)	// see it as a spike
	             break;
	          cwState = MODE_SPACE;
	          cwStartTime = currentTime;
	          add (MODE_TONE, duration);
	          break;
	      }
	      else 
	         last_peak = 0.98 * last_peak + 0.02 * value;
	      break;

	   case MODE_SPACE:
	      if (value > 0.8 * agc_peak) {
	         int duration = currentTime - cwStartTime;
	         if (duration < 3)	// see it as noise
	            break;
	         cwState	= MODE_TONE;
	         cwStartTime	= currentTime;
	         add (MODE_SPACE, duration);
	         last_peak	= value;
	         break;
	      }
	      spaceLevel = decayingAverage (spaceLevel, value, 50.0);
	      break;

	   default: ;
	}
}

bool	sorter (int i, int j) {
	return i < j;
}

#define	SEARCH_LENGTH	14
bool	isDot (int value, int norm) {
	if (value < 0)
	   fprintf (stderr, "Help 1");
	return value < 1.5 * norm;
}

bool	isLongSpace	(int value, int norm) {
	if (value > 0)
	   fprintf (stderr, "Help 2");
	return -value > norm;
}
//
//	The approach we take is to put the durations of the tones and spaces
//	into a queue. We know that a Morse symbol does not exceed 5
//	dots/dashes, so if the queue is sufficiently filled, we
//	try to figure out what for this combination the DOT
//	length would be.
void	elementHandler::add (int soort, int duration) {
int	bufferElems	= 0;

	buffer [fillerP] = soort == MODE_SPACE ? -duration : duration;
	fillerP		= (fillerP + 1) % QUEUE_LENGTH;
	bufferElems	= (fillerP + QUEUE_LENGTH - emptyP) % QUEUE_LENGTH;
	if (bufferElems < SEARCH_LENGTH)
	   return;
//
//	The first thing to do is to guess the length of the DOT
	std::vector<int> spaceSizes;
	std::vector<int> toneSizes;
	for (int i = 0; i < SEARCH_LENGTH; i ++) {
	   if (buffer [(emptyP + i) % QUEUE_LENGTH] < 0)
	      spaceSizes. push_back (-buffer [(emptyP + i) % QUEUE_LENGTH]);
	   else
	      toneSizes. push_back (buffer [(emptyP + i) % QUEUE_LENGTH]);
	}
	std::sort (spaceSizes. begin (), spaceSizes. end ());
	std::sort (toneSizes. begin (), toneSizes. end ());

	int spaceGuess	= spaceSizes. at (0);
	int dotGuess	= toneSizes. at (0);

	
//	since we know that a symbol takes at most 6 dash/dot combinations,
//	the longest space should be at least 2 times the shortest
//
	if (2 * spaceGuess > spaceSizes.  at (spaceSizes. size () - 1)) {
	   emptyP = (emptyP + 2) % QUEUE_LENGTH;
	   return;
	}
	WPM	= 600 / spaceGuess;
	if ((WPM < 5) || (WPM > 40)) {
	   emptyP = (emptyP + 2) % QUEUE_LENGTH;
	   return;
	}
	   
//	it is known that Morse symbols are at most 5 dash/dot combis
	for (int i = 0; i < 6; i ++) {
	   int space = -buffer [(emptyP + 2 * i + 1) % QUEUE_LENGTH];
	   if (space > 2.5 * spaceGuess) {
	      char bbb [10];
	      for (int j = 0; j <= i; j ++) {
	         int xx = buffer [(emptyP + 2 * j) % QUEUE_LENGTH];
//	         fprintf (stderr, "%d ", xx);
	         if (xx > 1.5 * spaceGuess)
	            bbb [j] = '_';
	         else
	            bbb [j] = '.';
	      }
	      bbb [i + 1] = 0;
	      char aaa [4];
	      lookupToken (bbb, aaa);
	      fprintf (stderr, "%s  %c\n", bbb, aaa [0]);
	      cw_addText (aaa [0]);
	      if (space > 3 * spaceGuess) {
	         cw_addText (' ');
	      }
	      emptyP = (emptyP + 2 * i + 2) % QUEUE_LENGTH;
	      return;
	   }
	}
	emptyP = (emptyP + 2) % QUEUE_LENGTH;
}
	
float	elementHandler::decayingAverage (float old, float input, float weight) {
	if (weight <= 1)
	   return input;
	return input * (1.0 / weight) + old * (1.0 - (1.0 / weight));
}


void 	elementHandler::printChar (char a, char er) {
	if ((a & 0x7f) < 32) {
	   switch (a) {
	      case '\n':		break;
	      case '\r':		return;
	      case 8:			break;
	      case 9:			break;
	      default:			a = ' ';
	   }
	}

	switch (er) {
	   case 0:	printf("%c",a);break;
	   case 1:	printf("\033[01;42m%c\033[m",a); break;
	   case 2:	printf("\033[01;41m%c\033[m",a); break;
	   case 3:	printf("\033[01;43m%c\033[m",a); break;
	   case 4:
	   case 5:
	   case 6:
	   case 7:	printf("\033[2J\033[H<BRK>\n"); break;
	   default:
			break;
	}
}

const char * const codetable[] = {
	"A._",
	"B_...",
	"C_._.",
	"D_..",
	"E.",
	"F.._.",
	"G__.",
	"H....",
	"I..",
	"J.___",
	"K_._",
	"L._..",
	"M__",
	"N_.",
	"O___",
	"P.__.",
	"Q__._",
	"R._.",
	"S...",
	"T_",
	"U.._",
	"V..._",
	"W.__",
	"X_.._",
	"Y_.__",
	"Z__..",
	"0_____",
	"1.____",
	"2..___",
	"3...__",
	"4...._",
	"5.....",
	"6_....",
	"7__...",
	"8___..",
	"9____.",
	".._._._",
	",__..__",
	"?..__..",
	"~"
	};

void	elementHandler::lookupToken (char *in, char *out) {
int	i;

	for (i = 0; codetable [i][0] != '~'; i ++) {
	   if (strcmp (&codetable [i][1], in) == 0) {
	      out [0] = codetable [i][0];
	      return;
	   }
	}
	out [0] = '*';
}

void	elementHandler::cw_clrText () {
	cwText		= std::string ("");
//	cwTextbox	-> setText (cwText);
}

void	elementHandler::cw_addText (char c) {
	if (c < ' ') c = ' ';
	cwText +=  char (c);
	if (cwText. length () > 83)
	   cwText. erase (0, 1);
	m_form -> setText (identity, cwFrequency, WPM, cwText);
}

void	elementHandler::reset	(int  n) {
	m_form -> setText (identity, 0, 0,  " ");
}

void	elementHandler::set_noiseLevel	(int n) {
	noiseLevel = n;
}

