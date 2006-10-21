// -*- C++ -*-
/**
 * \file xftFontMetrics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef XFT_FONT_METRICS_H
#define XFT_FONT_METRICS_H

#include "frontends/FontMetrics.h"

#include <gtkmm.h>
#include <X11/Xft/Xft.h>


namespace lyx {

class font_metrics: public lyx::frontend::FontMetrics
{
public:

	font_metrics(XftFont * f, XftFont * scf);

	virtual ~font_metrics() {}

	virtual int maxAscent() const;
	virtual int maxDescent() const;
	virtual int ascent(lyx::char_type c) const;
	int descent(lyx::char_type c) const;
	virtual int lbearing(lyx::char_type c) const;
	virtual int rbearing(lyx::char_type c) const;
	virtual int width(lyx::char_type const * s, size_t n) const;
	virtual int signedWidth(lyx::docstring const & s) const;
	virtual void rectText(lyx::docstring const & str,
		int & width,
		int & ascent,
		int & descent) const;
	virtual void buttonText(lyx::docstring const & str,
		int & width,
		int & ascent,
		int & descent) const;
private:
	XftFont * font;
	XftFont * fontS;
};

} // namespace lyx

#endif // XFT_FONT_METRICS_H
