// -*- C++ -*-
/**
 * \file MiniBuffer.h
 * Copyright 1995-2002 the LyX Team
 * Read the file COPYING
 *
 * \author Lars
 * \author Asger and Juergen
 */

#ifndef MINIBUFFER_H
#define MINIBUFFER_H

#include "LString.h"

#include <boost/signals/signal0.hpp>
#include <boost/signals/signal1.hpp>
#include <boost/signals/trackable.hpp>

#include <vector>

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class Timeout;

///
class MiniBuffer : public boost::signals::trackable {
public:
	///
	MiniBuffer(LyXView *);
	
	/// destructor
	virtual ~MiniBuffer();
	
	/// Displays a text for 6 seconds
	void message(string const & str);
	
	/**
	 * This will display a message for 6 seconds.
	 * It will remember the previous text that can be restored
	 * with messagePop. (You can only remember one message.)
	 */
	void messagePush(string const & str);
	
	/**
	 * Restore the previous text that was messagePush'ed.
	 * for 6 seconds
	 */
	void messagePop();
	
	/// Adds text to the text already displayed for 6 seconds
	void addSet(string const &);
	
	/** Makes the minibuffer wait for a string to be inserted.
	    Waits for a string to be inserted into the minibuffer, when
	    the string has been inserted the signal stringReady is
	    emitted.
	*/
	void prepareForInput(std::vector<string> const & completion,
			     std::vector<string> & history);
	
	/// This is signalled when the user has input a string
	boost::signal1<void, string const &> inputReady;
	
	/// This is signalled 6 seconds after a message has been displayed
	boost::signal0<void> timeout;
	
protected:
	/// Are we in editing mode?
	virtual bool isEditingMode() const = 0;
	/// enter editing mode
	virtual void editingMode() = 0;
	/// enter message display mode
	virtual void messageMode() = 0;
	
	/**
	 * This will show the info string for 1.5 seconds, after
	 * which it will revert to the input string.
	 * Use this in editing mode only. If the user presses a
	 * key in the 1.5 second interval, the information will
	 * disappear.
	 */
	void show_information(string const & info, string const & input);
	
	/**
	 * This is called after information has been shown for 1.5 seconds
	 * to restore the input as given in the show_information call.
	 */
	void restore_input();
	
	/**
	 * This is called when we tab-completed a command and it adds
	 * a space to the input so that we're ready to input any arguments.
	 */
	void set_complete_input(string const &);
	
	/// set the minibuffer content in editing mode
	virtual void set_input(string const &) = 0;
	
	/**
	 * This when a message has been displayed for 6 seconds and
	 * it will emit the timeout signal.
	 */
	void message_timeout();
	
	/**
	 * This will be the input after the information will timeout
	 * in 1.5 seconds or we press a key and force the information
	 * to disappear.
	 */
	string stored_input;
	
	/**
	 * This is true for 1.5 seconds while information is shown in
	 * editing mode. If the user presses a key while information
	 * is shown, the info will disappear.
	 */
	bool information_displayed_;
	
	///
	LyXView * owner_;
	
	/// This is the text for the message display
	string text;
	
	/// This is the last text after a messagePush()
	string text_stored;
	
	/**
	 * This will emit the timeout signal after a message has been
	 * displayed for 6 seconds.
	 */
	Timeout * timer;
	
	/**
	 * This will call restore_input after 1.5 seconds to restore
	 * the input after an information display.
	 */
	Timeout * information_timer_;
	
	///
	std::vector<string> completion_;
	///
	std::vector<string> * history_;
	///
	std::vector<string>::iterator hist_iter;
};
#endif
