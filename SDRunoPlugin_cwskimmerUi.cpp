#include <sstream>
#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/timer.hpp>
#include <unoevent.h>

#include "SDRunoPlugin_cwskimmer.h"
#include "SDRunoPlugin_cwskimmerUi.h"
#include "SDRunoPlugin_cwskimmerForm.h"

// Ui constructor - load the Ui control into a thread
	SDRunoPlugin_cwskimmerUi::
	             SDRunoPlugin_cwskimmerUi (SDRunoPlugin_cwskimmer& parent,
	                                       IUnoPluginController& controller) :
	m_parent(parent),
	m_form(nullptr),
	m_controller(controller) {
	m_thread = std::thread (&SDRunoPlugin_cwskimmerUi::ShowUi, this);
}

// Ui destructor (the nana::API::exit_all();) is required if using Nana UI library
	SDRunoPlugin_cwskimmerUi::~SDRunoPlugin_cwskimmerUi () {	
	nana::API::exit_all();
	m_thread.join();	
}

// Show and execute the form
void	SDRunoPlugin_cwskimmerUi::ShowUi () {	
	m_lock.lock();
	m_form = std::make_shared<SDRunoPlugin_cwskimmerForm> (*this,
	                                                       m_controller);
	m_lock.unlock();

	m_form->Run();
}

// Load X from the ini file (if exists)
// TODO: Change test to plugin name
int	SDRunoPlugin_cwskimmerUi::LoadX () {
std::string tmp;
	m_controller.GetConfigurationKey("cw-skimmer.X", tmp);
	if (tmp.empty()) {
	   return -1;
	}
	return stoi(tmp);
}

// Load Y from the ini file (if exists)
// TODO: Change test to plugin name
int	SDRunoPlugin_cwskimmerUi::LoadY () {
std::string tmp;
	m_controller.GetConfigurationKey ("cw-skimmer.Y", tmp);
	if (tmp.empty()) {
	   return -1;
	}
	return stoi(tmp);
}

// Handle events from SDRuno
// TODO: code what to do when receiving relevant events
void	SDRunoPlugin_cwskimmerUi::HandleEvent (const UnoEvent& ev) {
	switch (ev.GetType()) {
	case UnoEvent::StreamingStarted:
		break;

	case UnoEvent::StreamingStopped:
		break;

	case UnoEvent::SavingWorkspace:
		break;

	case UnoEvent::ClosingDown:
		FormClosed();
		break;

	default:
		break;
	}
}

// Required to make sure the plugin is correctly unloaded when closed
void	SDRunoPlugin_cwskimmerUi::FormClosed () {
	m_controller.RequestUnload(&m_parent);
}

void	SDRunoPlugin_cwskimmerUi::setText     (int index, int freq, int wpm, 
	                                       const std::string &s) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> setText (index, freq, wpm, s);
}

void	SDRunoPlugin_cwskimmerUi::handle_resetButton	() {
	m_parent. handle_resetButton ();
}

void	SDRunoPlugin_cwskimmerUi::set_width		(int n) {
	m_parent. set_width	(n);
}

void	SDRunoPlugin_cwskimmerUi::set_center		(int n) {
	m_parent. set_center	(n);
}

void	SDRunoPlugin_cwskimmerUi::reset_width		(int n) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> reset_width (n);
}

void	SDRunoPlugin_cwskimmerUi::reset_center		(int n) {
	std::lock_guard<std::mutex> l (m_lock);
	if (m_form != nullptr)
	   m_form -> reset_center (n);
}

