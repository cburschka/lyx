// -*- C++ -*-
/**
 * \file MiniBuffer.C
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars
 * \author Asger and Juergen
 */

#include <config.h>

#ifdef __GNUG__
#pragma implementation
#endif

#include "MiniBuffer.h"

#include "support/lyxalgo.h"
#include "support/filetools.h"
#include "support/lstrings.h"
#include "frontends/LyXView.h"
#include "gettext.h"
#include "LyXAction.h"
#include "BufferView.h"
#include "frontends/Timeout.h"

#include <boost/bind.hpp>

using std::vector;
using std::back_inserter;
using std::find;

extern LyXAction lyxaction;


MiniBuffer::MiniBuffer(LyXView * o)
	: information_displayed_(false), owner_(o)
{
	timer = new Timeout(6000);
	timer->timeout.connect(boost::bind(&MiniBuffer::message_timeout, this));
	
	information_timer_ = new Timeout(1500);
	information_timer_->timeout.connect(boost::bind(&MiniBuffer::restore_input, this));
}


MiniBuffer::~MiniBuffer()
{
	delete timer;
	delete information_timer_;
}


void MiniBuffer::show_information(string const & info, string const & input)
{
	stored_input = input;
	information_displayed_ = true;
	information_timer_->start();
	set_input(info);
}


void MiniBuffer::restore_input()
{
	if (information_displayed_) {
		information_displayed_ = false;
		set_input(stored_input);
	}
}


void MiniBuffer::message(string const & str)
{
	timer->restart();
	string const ntext = strip(str);
	if (! isEditingMode()) {
		set_input(ntext);
		text = ntext;
	}
}


void MiniBuffer::prepareForInput(vector<string> const & completion,
				 vector<string> & history)
{
	completion_ = completion;
	history_ = &history;
	hist_iter = history_->end();
	text.erase();
	set_input("");
	editingMode();
}


void MiniBuffer::message_timeout()
{
	// If we have focus, we don't want to change anything.
	if (isEditingMode())
		return;
	
	timeout();
	// we have redraw problems therefor we don't stop the
	// timer as so we force a redraw every 6 seconds.
	//	timer->stop();
}


void MiniBuffer::set_complete_input(string const & str)
{
	if (!str.empty()) {
		// add a space so the user can type
		// an argument immediately
		set_input(str + " ");
	}
}
