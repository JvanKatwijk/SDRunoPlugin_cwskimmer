
#ifndef	__ELEMENT_HANDLER_H
#define	__ELEMENT_HANDLER_H

#include	<stdint.h>
#include	<stdio.h>
#include		".\utilities.h"
#include	<cstring>
#include	<string>
#include "SDRunoPlugin_cwskimmerUi.h"

#define	MODE_IDLE		0100
#define	MODE_TONE		0200
#define	MODE_SPACE		0500

#define	CW_DOT_REPRESENTATION	'.'
#define	CW_DASH_REPRESENTATION	'_'
/*
 * 	The standard morse wordlength (i.e.
 * 	PARIS) is 50 bits, then for a wpm of 1,
 * 	one bit takes 1.2 second or 1.2 x 10^6	microseconds
 */

#define	QUEUE_LENGTH 32

#define	CW_RECEIVE_CAPACITY	040

class	elementHandler {
public:
		elementHandler	(SDRunoPlugin_cwskimmerUi *, int);
		~elementHandler	();
	void	reset		(int);
	void	reset		();
	void	process		(float value, int, float avg);
	void	set_noiseLevel	(int);
private:
	average		smoothenSamples;
	float		decayingAverage (float old,
	                                 float input, float weight);

	void	add		(int, int);
	SDRunoPlugin_cwskimmerUi	*m_form;
	int		identity;
	float		agc_peak;
	float		avg;
	float		spaceLevel;
	int		noiseLevel;
	float		value;
	float		last_peak;
	int		cwState;
	int		currentTime;
	int		cwCurrent;
	int		cwStartTime;
	int		starter;
	int		cwFrequency;
	std::string	cwText;
	int		WPM;
	int		fillerP;
	int		emptyP;
	int		buffer [QUEUE_LENGTH];


	void		cw_addText	(char c);
	void		cw_clrText	();
	void		lookupToken	(char *, char *);
	void		printChar	(char a, char err);

};
#endif

