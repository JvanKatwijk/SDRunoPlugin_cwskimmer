#pragma once

#include	<thread>
#include	<mutex>
#include	<atomic>
#include	<iunoplugincontroller.h>
#include	<iunoplugin.h>
#include	<iunostreamobserver.h>
#include	<iunoaudioobserver.h>
#include	<iunoaudioprocessor.h>
#include	<iunostreamobserver.h>
#include	<iunoannotator.h>

#include	"SDRunoPlugin_cwskimmerUi.h"
#include	"ringbuffer.h"
#include	<complex>

class elementHandler;

class SDRunoPlugin_cwskimmer : public IUnoPlugin,
	                          public IUnoStreamProcessor,
                                  public IUnoAudioProcessor {

	
public:
	
	SDRunoPlugin_cwskimmer (IUnoPluginController& controller);
	virtual ~SDRunoPlugin_cwskimmer ();

	virtual const
	char*	GetPluginName() const override { return "cw skimmer"; }

	void	StreamProcessorProcess (channel_t channel,
	                                Complex *buffer, int length,
	                                    bool& modified) override;
        void    AudioProcessorProcess (channel_t channel,
                                       float *buffer,
                                       int length, bool& modified) override;

	// IUnoPlugin
	virtual
	void HandleEvent		(const UnoEvent& ev) override;
	void	reset			();
	void	handle_resetButton	();
	void	set_width		(int);
	void	handle_threshold	(int);
	void	handle_modeSwitch	(int);

private:
	IUnoPluginController *m_controller;
	RingBuffer<std::complex<float>> inputBuffer;
	void			WorkerFunction();
	std::mutex		m_lock;
	std::mutex		processLocker;
	std::thread*		m_worker;
	SDRunoPlugin_cwskimmerUi m_form;
	std::atomic<bool> 	running;
	std::vector<elementHandler *> workVector;
	int			fftSize;
	int			width;
	int			center;
	int			lowEnd;
	int			highEnd;
	void			show_frequency	();
	int			the_threshold;
	float			the_avg;
};

