// -*- C++ -*-
#ifndef MINIBUFFER_H
#define MINIBUFFER_H

#include "LString.h"

#include <boost/signals/signal0.hpp>
#include <boost/signals/signal1.hpp>
#include <boost/signals/trackable.hpp>

#include <vector>

#include FORMS_H_LOCATION

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;
class DropDown;
class Timeout;

///
class MiniBuffer : public boost::signals::trackable {
public:
	enum State {
		spaces,
		nospaces
	};

	///
	MiniBuffer(LyXView * o,
		   FL_Coord x, FL_Coord y, FL_Coord h, FL_Coord w);

	/// destructor
	~MiniBuffer();

	/// create drop down
	void dd_init();

	///
	void addSet(string const &,
		    string const & = string());

	///
	void message(string const & str);
	///
	void messagePush(string const & str);
	///
	void messagePop();

	/** Makes the minibuffer wait for a string to be inserted.
	    Waits for a string to be inserted into the minibuffer, when
	    the string has been inserted the signal stringReady is
	    emitted.
	*/
	void getString(State space,
		       std::vector<string> const & completion,
		       std::vector<string> & history);
	///
	void redraw();
	///
	int peek_event(FL_OBJECT *, int, int);
	///
	boost::signal1<void, string const &> stringReady;
	///
	//boost::signal0<void> escape;
	///
	boost::signal0<void> timeout;
private:
	///
	void activate();
	///
	void deactivate();
	///
	void prepare();
	///
	void stored_slot();
	///
	void stored_set(string const &);
	/// set the minibuffer content if str non-empty
	void set_complete_input(string const &);
	/// append c to the current contents
	void append_char(char c);
	/// set the minibuffer content
	void set_input(string const &);
	///
	void init();
	///
	string stored_input;
	///
	bool stored_;
	///
	LyXView * owner_;
	///
	string text;
	///
	string text_stored;
	///
	FL_OBJECT * add(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord);
	///
	Timeout * timer;
	///
	Timeout * stored_timer;
	/// the dropdown menu
	DropDown * dropdown_;
	///
	FL_OBJECT * the_buffer;
	///
	std::vector<string> completion_;
	///
	std::vector<string> * history_;
	///
	std::vector<string>::iterator hist_iter;
	///
	State state_;
};
#endif
