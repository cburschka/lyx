// -*- C++ -*-
/* This file is part of
 * ====================================================== 
 *
 *           LyX, The Document Processor
 *
 *           Copyright 2001 The LyX Team.
 *
 * ======================================================
 *
 * \file ControlSpellchecker.h
 * \author Edwin Leuven <leuven@fee.uva.nl>
 */

#ifndef CONTROLSPELLCHECKER_H
#define CONTROLSPELLCHECKER_H

#ifdef __GNUG__
#pragma interface
#endif

#include "ControlDialogs.h"
#include "sp_base.h"

/** A controller for Spellchecker dialogs.
 */
class ControlSpellchecker : public ControlDialog<ControlConnectBD> {
public:
	///
	ControlSpellchecker(LyXView &, Dialogs &);

	/// replace word with replacement
	void replace(string const &);

	/// replace all occurances of word
	void replaceAll(string const &);

	/// insert word in personal dictionary
	void insert();

	/// ignore all occurances of word
	void ignoreAll();
	
	/// stop checking
	void stop();

	/// check text until next misspelled/unknown word
	void check();

	/// spell options
	void options();
	
	/// get suggestion
	string getSuggestion();

	/// get word
	string getWord();

	/// returns progress value
	int getProgress() {
		return oldval_; 
	}

	/// returns exit message
	string getMessage() {
		return message_;
	}
	
private:
	
	/// set the params before show or update
	void setParams();
	/// clean-up on hide.
	void clearParams();

	/// not needed.
	virtual void apply() {}

	/// right to left
	bool rtl_;

	/// current word being checked
	string word_;

	/// values for progress
	float newval_;
	int oldval_;
	int newvalue_;

	/// word count
	int count_;

	/// exit message
	string message_;

	/// set to true to stop checking
	bool stop_;

	/// spellchecker status
	enum SpellBase::spellStatus result_;

	/// The actual spellchecker object
	SpellBase * speller_;
	
};

#endif // CONTROLSPELLCHECKER_H
