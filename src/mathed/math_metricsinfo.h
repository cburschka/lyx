#ifndef MATH_METRICSINFO_H
#define MATH_METRICSINFO_H

#include "lyxfont.h"

class BufferView;
class MathNestInset;


/// Standard Math Sizes (Math mode styles)
enum MathStyles {
	///
	LM_ST_DISPLAY = 0,
	///
	LM_ST_TEXT,
	///
	LM_ST_SCRIPT,
	///
	LM_ST_SCRIPTSCRIPT
};


struct MathMetricsInfo {
	///
	MathMetricsInfo()
		: view(0), font(), style(LM_ST_TEXT),
		  inset(0), idx(0),
		  fullredraw(false)
	{}
	///
	MathMetricsInfo(BufferView * v, LyXFont const & f, MathStyles s)
		: view(v), font(f), style(s),
		  inset(0), idx(0),
		  fullredraw(false)
	{}

	///
	BufferView * view;
	///
	LyXFont font;
	///
	MathStyles style;
	/// used to pass some info down
	MathNestInset const * inset;
	///
	int idx;
	///
	bool fullredraw;
};


struct TextMetricsInfo {
	///
	TextMetricsInfo()
		{}
	/// used to pass some info down
	MathNestInset const * inset;
	///
	int idx;
};

#endif
