
#include	"element-handler.h"
#include	<stdio.h>
//#include	<algorithm>
#include	<cstring>
#include	<vector>
//#include	"radio.h"

#include	"SDRunoPlugin_cwskimmerUi.h"


static inline
float   get_db (float x) { 
        return 20 * log10 ((x + 0.01) / (float)(256));
}

//
//	The samplerate is 500 Ss, i.e. 2 millisecond per sample 
//	the number of elements for PARIS is 50, so with 
//	a rate of 13 words per minute, we have 13 * 50 = 650 elements
//	per minute, i.e. 92.31 millisecond, i.e. 46 samples.
//	For 30 words a minute 30 * 50 elements per minute, i.e.
//	1500 elements in 60000 milliseconds, i.e. 40 milliseconds
//	adding up to 20 samples.
//
	elementHandler::elementHandler (SDRunoPlugin_cwskimmerUi *mr,
	                                int ident): 
	                                    smoothenSamples (4) {
	this	-> m_form	= mr;
	this	-> identity	= ident;
	reset	();
}

	elementHandler::~elementHandler () {}

void	elementHandler::reset	() {
	cwCurrent		= 0;
	peakLevel		= 0;
	noiseLevel		= 0;
	currentPeak		= 0;
	avg			= 0;
	starter			= 0;
	cwState			= MODE_IDLE;
	cwText			= std::string ("");
//
//	times etc expressed in samples
	currentTime		= 0;
	fillerP			= 0;
	emptyP			= 0;
}

void	elementHandler::process	(float value, int freq, float threshold) {

	value = smoothenSamples.filter (value);
	if (starter < 500) {
	   avg	= decayingAverage (avg, value, 500);
	   peakLevel = 2 * avg;
	   starter++;
	   return;
	}

	if (value > 2 * avg)
	   peakLevel = decayingAverage (peakLevel, value, 50.0);

	this	-> cwFrequency	= freq;
//
//	the incoming samplerate is 1000 / 2 samples/second,
//	i.e. app 500 
	currentTime	+= 1;

	switch (cwState) {
	   case MODE_IDLE:
	      if ((value > 0.67 * peakLevel)  &&
	          (get_db (value) > get_db (avg) + threshold)) {
	         currentPeak	= value;
	         cwState	= MODE_IN_TONE;
	         cwStartTime	= currentTime;;
	         peakLevel	= decayingAverage (peakLevel, value, 100.0);
	      }
	      else
	         avg = decayingAverage (avg, value, 1000);
	      break;

	   case MODE_IN_TONE:
	      if (value > 2 * avg) {	// stay in tone
	         if (value > currentPeak)
	            currentPeak = value;
	         peakLevel	= decayingAverage (peakLevel, value, 100.0);
	         break;			/* just go on	*/
	      }
	      
	      if (currentTime - cwStartTime < 3)	// a spike?
	         break;		
	
	      add (MODE_IN_TONE, currentTime - cwStartTime);
	      cwState		= MODE_SPACE;
	      cwStartTime	= currentTime;
	      avg = decayingAverage (avg, value, 1000);
	      break;

	   case MODE_SPACE:
	      if ((value >  0.67 * peakLevel ) &&
	          (get_db (value) > get_db (avg) + threshold)) {
	         if (currentTime - cwStartTime < 3)	// see it as noise
	            break;
	         currentPeak		= value;
	         add (MODE_SPACE, currentTime - cwStartTime);
	         cwState		= MODE_IN_TONE;
	         cwStartTime		= currentTime;
	         peakLevel		= decayingAverage (peakLevel,
	                                              value, 100.0);
	      }
	      else
	         avg = decayingAverage (avg, value, 1000);
	      break;

	   default: ;
	}
}


#define	SEARCH_LENGTH	14
//
//	The approach we take is to put the durations of the tones and spaces
//	into a queue. We know that a Morse symbol does not exceed 5
//	dots/dashes, so if the queue is sufficiently filled, we
//	try to figure out what for this combination the DOT
//	length would be.
void	elementHandler::add (int soort, int duration) {
int	bufferElems	= 0;
	
	if (duration == 0) {
	   m_form -> setText (identity, cwFrequency, WPM, "foute boel");
	   reset (0);
	}

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

	if ((spaceSizes. size () == 0) || (toneSizes. size () == 0)) {
	   m_form -> setText (identity, cwFrequency, WPM, "foute boel");
           reset (1);
	   return;
	}
	std::sort (spaceSizes. begin (), spaceSizes. end ());
	std::sort (toneSizes. begin (), toneSizes. end ());

	int spaceGuess	= spaceSizes. at (0);
	int dotGuess	= toneSizes. at (0);

	if ((dotGuess < 10) || (spaceGuess < 10)) {
	   emptyP = (emptyP + 2) % QUEUE_LENGTH;
	   return;
        }

	float ratio     = (float)dotGuess / (float)spaceGuess;
        if ((ratio < 0.5) || (ratio > 1.5)) {   // garbage
           emptyP = (emptyP + 2) % QUEUE_LENGTH;
           return;
        }

//	since we know that a symbol takes at most 6 dash/dot combinations,
//	the longest space should be at least 2 times the shortest
//
	
	if (2 * spaceGuess > spaceSizes.  at (spaceSizes. size () - 1)) {
	   emptyP = (emptyP + 2) % QUEUE_LENGTH;
	   return;
	}
	
	std::vector<int> message;
	for (int i = 0; i < QUEUE_LENGTH / 2; i ++) {
	   message . push_back (buffer [(emptyP + 2 * i) % QUEUE_LENGTH]);
	   message . push_back ( - buffer [(emptyP + 2 * i + 1) % QUEUE_LENGTH]);
	   if (message [2 * i + 1] > 2.5 * spaceGuess)
	      break;
	} 
	if (message. size () == QUEUE_LENGTH) {
	   emptyP = (emptyP + 2) % QUEUE_LENGTH;
	   return;	// seems garbage only
	}
//
//	Since the border between the signal strengths denoting a mark
//	and the ones denoting a space is not hard, we use the
//	actual size of the space as indicator and compute
//	from there what the size of a dot should be
	int spaceSize = 0;
	for (int i = 0; i < message. size () / 2 - 1; i ++)
	   spaceSize += message [2 * i + 1];
	if (spaceSize == 0)	// only a dot or a dash
	   spaceSize = spaceGuess;
	else
	   spaceSize /= message. size () / 2 - 1;

	float dotCorrection = (float)spaceSize / dotGuess;

	WPM	= 600 / spaceSize;
	if ((WPM < 5) || (WPM > 45)) {
	   emptyP = (emptyP + 2) % QUEUE_LENGTH;
	   return;
	}
//	it is known that Morse symbols are at most 5 dash/dot combis
	for (int i = 0; i < 6; i ++) {
	   int space = -buffer [(emptyP + 2 * i + 1) % QUEUE_LENGTH];
	   if (space > 2 * spaceSize) {	// end of token
	      char bbb [10];
	      for (int j = 0; j <= i; j ++) {
	         int dataElem = buffer [(emptyP + 2 * j) % QUEUE_LENGTH];
	         if (dataElem * dotCorrection > 2 * spaceGuess)
	            bbb [j] = '_';
	         else
	            bbb [j] = '.';
	      }
	      bbb [i + 1] = 0;
	      char aaa [4];
	      lookupToken (bbb, aaa);
	      cw_addText (aaa [0]);
	      if (space > 4 * spaceSize) {
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
}

