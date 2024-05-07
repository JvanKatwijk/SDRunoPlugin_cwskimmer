#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <iunoplugin.h>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>
#include "SDRunoPlugin_cwskimmerForm.h"

// Forward reference
class SDRunoPlugin_cwskimmer;

class	SDRunoPlugin_cwskimmerUi {
public:

	SDRunoPlugin_cwskimmerUi (SDRunoPlugin_cwskimmer& parent,
	                          IUnoPluginController& controller);
	~SDRunoPlugin_cwskimmerUi();

	void HandleEvent	(const UnoEvent& evt);
	void FormClosed		();

	void ShowUi();

	int LoadX();
	int LoadY();

	void	setText			(int, int, int, const std::string &);
	void	handle_resetButton	();
	void	set_width		(int);
	void	handle_modeSwitch	(int);
	void	handle_threshold	(int);
//
//	going down:
	void	reset_width		(int);
	void	show_frequencyRange	(int, int);

private:
	
	SDRunoPlugin_cwskimmer & m_parent;
	std::thread m_thread;
	std::shared_ptr<SDRunoPlugin_cwskimmerForm> m_form;

	bool m_started;

	std::mutex m_lock;

	IUnoPluginController & m_controller;
};
