#include	<sstream>
#include	<unoevent.h>
#include	<iunoplugincontroller.h>
#include	<vector>
#include	<sstream>
#include	<chrono>
#include	<Windows.h>
#include	"SDRunoPlugin_cwskimmer.h"
#include	"SDRunoPlugin_cwskimmerUi.h"

#include	"element-handler.h"
#include	"fft-complex.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846 
#endif

#define	NR_ELEMENTS	32
#define FRAGMENT	(2 * 192)
	SDRunoPlugin_cwskimmer::
	      SDRunoPlugin_cwskimmer (IUnoPluginController& controller):
	                                      IUnoPlugin (controller),
	                                      m_form (*this, controller),
	                                      m_worker (nullptr),
	                                      inputBuffer (8 * 32768) {
	m_controller	= &controller;
	running. store (false);
	m_controller    -> RegisterAudioProcessor (0, this);
	m_controller    -> SetDemodulatorType (0,
	                         IUnoPluginController::DemodulatorIQOUT);


	this	-> width	= 3;
	this	-> center	= NR_ELEMENTS / 2;
	this	-> lowEnd	= center - 1;;
	this	-> highEnd	= center + 1;
	fftSize			= 1024;

	the_avg			= 0;
	the_threshold		= 0;
	workVector. resize (NR_ELEMENTS);
	for (int i = 0; i < NR_ELEMENTS; i ++)
	   workVector [i] = new elementHandler (&m_form, i);
	m_worker        =
	         new std::thread (&SDRunoPlugin_cwskimmer::WorkerFunction, this);
}

	SDRunoPlugin_cwskimmer::~SDRunoPlugin_cwskimmer () {
	running. store (false);
	m_worker -> join ();
//      m_controller    -> UnregisterStreamProcessor (0, this);
	m_controller    -> UnregisterAudioProcessor (0, this);
	for (int i = 0; i < NR_ELEMENTS; i ++)
	   delete workVector [i];

}

void	SDRunoPlugin_cwskimmer::HandleEvent (const UnoEvent& ev) {
	switch (ev. GetType ()) {
           case UnoEvent::FrequencyChanged:
	      show_frequency ();
	      break;

           case UnoEvent::CenterFrequencyChanged:
              break;

           default:
              m_form. HandleEvent (ev);
              break;
        }
}

void    SDRunoPlugin_cwskimmer::StreamProcessorProcess (channel_t channel,
	                                                Complex *buffer,
	                                                int length,
	                                                bool& modified) { 
	(void)channel; (void)buffer; (void)length;
	modified = false;
}

void    SDRunoPlugin_cwskimmer::AudioProcessorProcess (channel_t channel,
	                                               float* buffer,
	                                               int length,
	                                               bool& modified) {
///      Handling IQ input, note that SDRuno interchanges I and Q elements

        if (!modified) {
           for (int i = 0; i < length; i ++) {
              std::complex<float> sample =
                           std::complex<float>(buffer [2 * i + 1],
                                               buffer [2 * i]);
              inputBuffer. putDataIntoBuffer (&sample, 1);
           }
        }
}

void	SDRunoPlugin_cwskimmer::WorkerFunction	() {
std::vector<std::complex<float>> segment (fftSize);
std::vector<std::complex<float>> ftBuffer (fftSize);
std::vector<float> blackmanWindow (fftSize);
float	l_avg	= 0;
	inputBuffer.FlushRingBuffer();
	for (int i = 0; i < fftSize; i ++)
	   blackmanWindow [i] = 6 * (0.42 -
	              0.50 * cos(2 * M_PI * (float)i / (float)fftSize) +
	              0.08 * cos(4 * M_PI * (float)i / (float)fftSize));
	int	binWidth	= 192000.0 / fftSize;
	reset();
	show_frequency();
	running. store (true);
	while (running. load ()) {
	   while (running.load() &&
			(inputBuffer.GetRingBufferReadAvailable () < 2 * 192))
	      Sleep (1);

//	we seem to have data, apply FFT
	   for (int i = 0; i < fftSize - 2 * 192; i ++)
	      ftBuffer [i] = ftBuffer [2 * 192 + i];
	   inputBuffer.getDataFromBuffer (&ftBuffer [fftSize - 2 * 192],
	                                              2 * 192);

	   for (int i = 0; i < fftSize; i ++)
	      segment [i] = ftBuffer [i] * blackmanWindow [i];
	   
	   Fft_transform (segment. data (), fftSize, false);
	   for (int i = this -> lowEnd; i <= this -> highEnd; i ++)
	      l_avg += abs (segment [(fftSize + (i - center)) % fftSize]);
	   the_avg = 0.99 * the_avg + 0.01 * l_avg / (highEnd - lowEnd + 1);
	   int selectedFreq = m_controller -> GetVfoFrequency (0);

//	   processLocker. lock ();
	   for (int i = this -> lowEnd; i <= this -> highEnd; i ++) {
	      int freq = selectedFreq + (i - center) * binWidth;
	      int index	= (fftSize + (i - center)) % fftSize;
		  float sample = 2 * abs(segment [index]);
	      workVector [i] -> process (sample, freq, the_threshold);
	   }
//	   processLocker. unlock ();
	}
}

void	SDRunoPlugin_cwskimmer::set_width	(int n) {

	if ((n & 01) != 0) {	// odd
	   lowEnd	= center - n / 2;
	   highEnd	= center + n / 2;
	}
	else {
	   lowEnd	= center - n / 2;
	   highEnd	= center + n / 2 - 1;
	}
	this	-> width	= n;
	show_frequency ();
//	processLocker. unlock ();
}

void	SDRunoPlugin_cwskimmer::reset	() {
	for (int i = 0; i < NR_ELEMENTS; i ++)
	   workVector [i] -> reset ();
	the_avg	= 0;
}

void	SDRunoPlugin_cwskimmer::handle_resetButton	() {
	reset ();

	for (int i = 0; i < NR_ELEMENTS; i ++)
	   m_form. setText (i, 0, 0, "   ");

}

void	SDRunoPlugin_cwskimmer::handle_threshold	(int n) {
	the_threshold	= n;
}
void	SDRunoPlugin_cwskimmer::show_frequency		() {
int	selectedFreq	= m_controller -> GetVfoFrequency (0);
int	binWidth	= 192000.0 / fftSize;
int	lowFreq		= selectedFreq + (lowEnd - center) * binWidth;
int	highFreq	= selectedFreq + (highEnd - center) * binWidth;

	m_form. show_frequencyRange (lowFreq / 1000, highFreq / 1000);
}

void	SDRunoPlugin_cwskimmer::handle_modeSwitch (int s) {
	running.store(false);
	m_worker -> join ();
//	delete m_worker;
	fftSize	= s;
	m_worker        =
	         new std::thread (&SDRunoPlugin_cwskimmer::WorkerFunction, this);
}

