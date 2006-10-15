// -*- C++ -*-
/**
 * \file NoGuiFontMetrics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef LYX_NO_GUI_FONT_METRICS_H
#define LYX_NO_GUI_FONT_METRICS_H

#include "frontends/FontMetrics.h"

#include "support/docstring.h"

namespace lyx {
namespace frontend {

class NoGuiFontMetrics: public FontMetrics
{
public:

	NoGuiFontMetrics() {}

	virtual ~NoGuiFontMetrics() {}

	virtual int maxAscent() const { return 1; }

	virtual int maxDescent() const { return 1; }
	
	virtual int ascent(lyx::char_type c) const { return 1; }
	
	int descent(lyx::char_type c) const { return 1; }
	
	virtual int lbearing(lyx::char_type c) const { return 1; }
	
	virtual int rbearing(lyx::char_type c) const { return 1; }
	
	virtual int width(lyx::char_type const * s, size_t n) const { return n; }
	
	virtual int signedWidth(lyx::docstring const & s) const
	{
		if (s[0] == '-')
			return -FontMetrics::width(s.substr(1, s.length() - 1));
		else
			return FontMetrics::width(s);
	}
	
	virtual void rectText(lyx::docstring const & str,
		int & width,
		int & ascent,
		int & descent) const {};	 
	
	virtual void buttonText(lyx::docstring const & str,
		int & width,
		int & ascent,
		int & descent) const {};
};

} // namespace frontend
} // namespace lyx

#endif // LYX_NO_GUI_FONT_METRICS_H
