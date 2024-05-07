#pragma once

#include <nana/gui.hpp>
#include <nana/gui/widgets/button.hpp>
#include <nana/gui/widgets/listbox.hpp>
#include <nana/gui/widgets/slider.hpp>
#include <nana/gui/widgets/label.hpp>
#include <nana/gui/widgets/combox.hpp>
#include <nana/gui/widgets/spinbox.hpp>
#include <nana/gui/timer.hpp>
#include <nana/gui/widgets/picture.hpp>
#include <nana/gui/filebox.hpp>
#include <nana/gui/dragger.hpp>
#include <iostream>
#include <iomanip>
#include <sstream>

#include <iunoplugincontroller.h>

// Shouldn't need to change these
#define topBarHeight (27)
#define bottomBarHeight (8)
#define sideBorderWidth (8)

// TODO: Change these numbers to the height and width of your form
#define formWidth (800)
#define formHeight (700)

class SDRunoPlugin_cwskimmerUi;

class SDRunoPlugin_cwskimmerForm : public nana::form {
public:

	SDRunoPlugin_cwskimmerForm (SDRunoPlugin_cwskimmerUi& parent,
	                            IUnoPluginController& controller);		
	~SDRunoPlugin_cwskimmerForm();
	void Run		();
	void	setText		(int, int, int, const std::string &);
	void	handle_resetButton	();
	void	set_width	(int);
	void	set_modeSelector	(const std::string &);
	void	handle_threshold	(int);
//
//	and downwards
	void	reset_width	(int);
	void	reset_center	(int);
	void	show_frequencyRange	(int, int);
	
private:

	void Setup	();

	// The following is to set up the panel graphic to look like a standard SDRuno panel
	nana::picture bg_border{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::picture bg_inner{ bg_border, nana::rectangle(sideBorderWidth, topBarHeight, formWidth - (2 * sideBorderWidth), formHeight - topBarHeight - bottomBarHeight) };
	nana::picture header_bar{ *this, true };
	nana::label title_bar_label{ *this, true };
	nana::dragger form_dragger;
	nana::label form_drag_label{ *this, nana::rectangle(0, 0, formWidth, formHeight) };
	nana::paint::image img_min_normal;
	nana::paint::image img_min_down;
	nana::paint::image img_close_normal;
	nana::paint::image img_close_down;
	nana::paint::image img_header;
	nana::picture close_button{ *this, nana::rectangle(0, 0, 20, 15) };
	nana::picture min_button{ *this, nana::rectangle(0, 0, 20, 15) };

	// Uncomment the following 4 lines if you want a SETT button and panel
	nana::paint::image img_sett_normal;
	nana::paint::image img_sett_down;
	nana::picture sett_button{ *this, nana::rectangle(0, 0, 40, 15) };
	void SettingsButton_Click();

	// TODO: Now add your UI controls here

	SDRunoPlugin_cwskimmerUi & m_parent;
	IUnoPluginController & m_controller;
                                    
        std::string messageLabel;
	nana::spinbox	width_setter	{*this,
	                           nana::rectangle (40, 30, 80, 50)};
	nana::button	resetButton	{*this,
	                           nana::rectangle (140, 30, 50, 50)};
	nana::spinbox	threshold	{ *this,
				   nana::rectangle (210, 30, 80, 50) };
	nana::combox	modeSelector	{*this,
				   nana::rectangle (310, 30, 80, 50) };
	nana::label	minFreq		{*this,
				   nana::rectangle (410, 30, 60, 50) };
	nana::label	maxFreq		{*this,
				   nana::rectangle (480, 30, 60, 50) };
	nana::listbox	list	{*this,
	                         nana::rectangle (30, 90, 750, 600)};
};
