#ifndef MATH_METRICSINFO
#define MATH_METRICSINFO

#include "lyxfont.h"

class BufferView;


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
		: view(0), font(), style(LM_ST_TEXT)
	{}
	///
	MathMetricsInfo(BufferView * v, LyXFont const & f, MathStyles s)
		: view(v), font(f), style(s)
	{}

	///
	BufferView * view;
	///
	LyXFont font;
	///
	MathStyles style;
};

#endif
