// -*- C++ -*-
#ifndef MINIBUFFER_H
#define MINIBUFFER_H

#include <sigc++/signal_system.h>
#include <vector>

#include FORMS_H_LOCATION
#include "LString.h"
#include "frontends/Timeout.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;

///
class MiniBuffer : public SigC::Object {
public:
	enum State {
		spaces,
		nospaces
	};
	
	///
	MiniBuffer(LyXView * o,
		   FL_Coord x, FL_Coord y, FL_Coord h, FL_Coord w);

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
	    the string has been insterted the signal stringReady is
	    emitted.
	*/
	void getString(State space,
		       vector<string> const & completion,
		       vector<string> & history);
	///
	void redraw();
	///
        int peek_event(FL_OBJECT *, int, int);
	///
	SigC::Signal1<void, string const &> stringReady;
	///
	//SigC::Signal0<void> escape;
	///
	SigC::Signal0<void> timeout;
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
	Timeout timer;
	///
	Timeout stored_timer;
	///
	FL_OBJECT * the_buffer;
	///
	vector<string> completion_;
	///
	vector<string> * history_;
	///
	vector<string>::iterator hist_iter;
	///
	State state_;
};
#endif
