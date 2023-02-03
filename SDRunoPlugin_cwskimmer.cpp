#include <sstream>
#include <unoevent.h>
#include <iunoplugincontroller.h>
#include <vector>
#include <sstream>
#include <chrono>
#include	<Windows.h>
#include "SDRunoPlugin_cwskimmer.h"
#include "SDRunoPlugin_cwskimmerUi.h"

#include "element-handler.h"
#include	"fft-complex.h"

#define	FFT_SIZE	2048
#define	NR_ELEMENTS	32
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

	workVector. resize (NR_ELEMENTS);
	this	-> width	= 3;
	this	-> center	= NR_ELEMENTS / 2;
	this	-> lowEnd	= center - 1;;
	this	-> highEnd	= center + 1;
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

}

void	SDRunoPlugin_cwskimmer::HandleEvent (const UnoEvent& ev) {
	m_form.HandleEvent(ev);	
}

void    SDRunoPlugin_cwskimmer::StreamProcessorProcess (channel_t channel,
	                                                Complex *buffer,
	                                                int length,
	                                                bool& modified) { 
	(void)channel; (void)buffer; (void)length;
//	modified = false;
}

void    SDRunoPlugin_cwskimmer::AudioProcessorProcess (channel_t channel,
	                                               float* buffer,
	                                               int length,
	                                               bool& modified) {
///      Handling IQ input, note that SDRuno interchanges I and Q elements

	if (!modified) {
	   inputBuffer.putDataIntoBuffer (buffer, length);
	}
}

void	SDRunoPlugin_cwskimmer::WorkerFunction	() {
std::complex<float> ftBuffer [FFT_SIZE];

	int	binWidth	= 192000.0 / FFT_SIZE;
	running. store (true);
	while (running. load ()) {
	   while (running.load() &&
			(inputBuffer.GetRingBufferReadAvailable () < 2 * 192))
	      Sleep (1);

//	we seem to have data, apply FFT
	   int selectedFreq = m_controller -> GetVfoFrequency (0);
	   inputBuffer.getDataFromBuffer(ftBuffer, 2 * 192);
	   for (int i = 2 * 192; i < FFT_SIZE; i++)
	      ftBuffer [i] = std::complex<float>(0, 0);
	   Fft_transform (ftBuffer, FFT_SIZE, false);
//

	   for (int i = this -> lowEnd; i <= this -> highEnd; i ++) {
	      int freq = selectedFreq + (i - center) * binWidth;
	      int index	= (FFT_SIZE + (i - center)) % FFT_SIZE;
	      workVector [i] -> process (abs (ftBuffer [index]), freq, 0);
	   }
	}
}

void	SDRunoPlugin_cwskimmer::set_width	(int n) {
int lowEnd_new, highEnd_new;

	if (center < n / 2)
	   return;
	if (center >= NR_ELEMENTS - n / 2)
	   return;

	if ((n & 01) != 0) {	// odd
	   lowEnd_new	= center - n / 2;
	   highEnd_new	= center + n / 2;
	}
	else {
	   lowEnd_new	= center - n / 2;
	   highEnd_new	= center + n / 2 - 1;
	}

	if (lowEnd_new > this -> lowEnd)
	   for (int i = this -> lowEnd; i < lowEnd_new; i++)
	      workVector [i] -> reset (0);

	if (highEnd_new < this -> highEnd)
	   for (int i = highEnd_new + 1; i <= this -> highEnd; i ++)
	      workVector [i] -> reset (0);
	this	-> width	= n;
	this	-> lowEnd	= lowEnd_new;
	this	-> highEnd	= highEnd_new;
}

void	SDRunoPlugin_cwskimmer::set_center (int n) { 
int	lowEnd_new, highEnd_new;

	if (n < width / 2)
	   return;
	if (n >= NR_ELEMENTS - width / 2)
	   return;

	if ((this -> width & 01) != 0) {	// odd
	   lowEnd_new	= n - this -> width / 2;
	   highEnd_new	= n + this -> width / 2;
	}
	else {
	   lowEnd_new	= n - this -> width / 2;
	   highEnd_new	= n + this -> width / 2 - 1;
	}

	if (lowEnd_new > this -> lowEnd)
	   for (int i = this -> lowEnd; i < lowEnd_new; i++)
	      workVector [i] -> reset (0);

	if (highEnd_new < this -> highEnd)
	   for (int i = highEnd_new + 1; i <= this -> highEnd; i ++)
	      workVector [i] -> reset (0);
	this	-> center	= n;
	this	-> lowEnd	= lowEnd_new;
	this	-> highEnd	= highEnd_new;
}


void	SDRunoPlugin_cwskimmer::reset	() {
	for (int i = 0; i < NR_ELEMENTS; i ++)
	   workVector [i] -> reset ();
}

void	SDRunoPlugin_cwskimmer::handle_resetButton	() {
	reset ();

	for (int i = 0; i < NR_ELEMENTS; i ++)
	   m_form. setText (i, 0, 0, "   ");

}

