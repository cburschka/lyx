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

#include "Dimension.h"

#include "support/strfwd.h"

namespace lyx {
namespace frontend {

class NoGuiFontMetrics : public FontMetrics
{
public:

	NoGuiFontMetrics() {}

	virtual ~NoGuiFontMetrics() {}

	virtual int maxAscent() const { return 1; }

	virtual int maxDescent() const { return 1; }

	virtual Dimension const defaultDimension() const { return Dimension(1, 1, 1); }

	virtual int width(char_type) const { return 1; }

	virtual int ascent(char_type) const { return 1; }

	int descent(char_type) const { return 1; }

	virtual int lbearing(char_type) const { return 1; }

	virtual int rbearing(char_type) const { return 1; }

	virtual int width(docstring const & s) const { return s.size(); }

	virtual int signedWidth(docstring const & s) const
	{
		if (s.size() && s[0] == '-')
			return -width(s.substr(1, s.length() - 1));
		return width(s);
	}

	virtual Dimension const dimension(char_type) const { return Dimension(1, 1, 1); }

	virtual void rectText(docstring const &,
		int & /*width*/, int & /*ascent*/, int & /*descent*/) const {}

	virtual void buttonText(docstring const &,
		int & /*width*/, int & /*ascent*/, int & /*descent*/) const {}
};

} // namespace frontend
} // namespace lyx

#endif // LYX_NO_GUI_FONT_METRICS_H
