// -*- C++ -*-
#ifndef _MINIBUFFER_H
#define _MINIBUFFER_H

#include FORMS_H_LOCATION
#include "LString.h"
#include "gettext.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXView;

///
class MiniBuffer {
public:
	///
	MiniBuffer(LyXView *o, FL_Coord x, FL_Coord y, FL_Coord h, FL_Coord w)
		: owner(o)
	{
		text = _("Welcome to LyX!");
		shows_no_match = true;
	        history_idx = history_cnt = 0;
	        add(FL_NORMAL_INPUT, x, y, h, w);
	}
	///
	bool shows_no_match;
	///
	void setTimer(int a) {
		fl_set_timer(timer, a);
	}
	///
	void Set(LString const& = LString(),
		 LString const& = LString(),
		 LString const& = LString(),
		 int delay_secs=6);
	/// 
	LString GetText() const { return text; }
	///
	void Init();
	///
	void ExecCommand();
	/** allows to store and reset the contents one time. Usefull
	  for status messages like "load font" (Matthias)
	  */
	void Store();
	///
	void Reset();
	///
	void Activate();
	///
	void Deactivate();
private:
	///
	LyXView *owner;
	///
	LString text;
	///
	LString text_stored;
	///
	FL_OBJECT *add(int, FL_Coord, FL_Coord, FL_Coord, FL_Coord);
	///
	static void ExecutingCB(FL_OBJECT *ob, long);
	///
	static void TimerCB(FL_OBJECT *ob, long);
	///
        static int  peek_event(FL_OBJECT *, int, FL_Coord, FL_Coord,
			       int, void *);
        ///
	FL_OBJECT *timer;
	///
	FL_OBJECT *the_buffer;
	///
	LString cur_cmd;
        ///
        enum{ MAX_HISTORY = 10 };
        ///
        LString history[MAX_HISTORY];
        ///
        int history_idx, history_cnt;
        ///
        void addHistory(LString const &cmd) { 
	        if (history_cnt==0 || (history_cnt>0 && cmd!=history[(history_cnt-1) % MAX_HISTORY])) {
		    history[history_cnt % MAX_HISTORY] = cmd;
		    history_cnt++;
		}
	        history_idx = history_cnt;
	}
        ///
        LString getHistory() { return history[history_idx % MAX_HISTORY]; }
};
#endif
