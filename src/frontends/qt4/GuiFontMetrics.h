// -*- C++ -*-
/**
 * \file GuiFontMetrics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUI_FONT_METRICS_H
#define GUI_FONT_METRICS_H

#include "frontends/FontMetrics.h"

#include "support/docstring.h"

#include <map>

#include <QFontMetrics>
#include <QHash>

namespace lyx {
namespace frontend {

class GuiFontMetrics : public FontMetrics
{
public:
	GuiFontMetrics(QFont const & font);

	virtual ~GuiFontMetrics() {}

	virtual int maxAscent() const;
	virtual int maxDescent() const;
	virtual Dimension const defaultDimension() const;
	virtual int width(char_type c) const;
	virtual int ascent(char_type c) const;
	virtual int descent(char_type c) const;
	virtual int lbearing(char_type c) const;
	virtual int rbearing(char_type c) const;
	virtual int width(docstring const & s) const;
	virtual int signedWidth(docstring const & s) const;
	virtual Dimension const dimension(char_type c) const;

	virtual void rectText(docstring const & str,
		int & width,
		int & ascent,
		int & descent) const;
	virtual void buttonText(docstring const & str,
		int & width,
		int & ascent,
		int & descent) const;
	///
	int width(QString const & str) const;

private:
	/// Metrics on the font
	QFontMetrics metrics_;

	/// Cache of char widths
	mutable QHash<char_type, int> width_cache_;

	/// Cache of string widths
	/// FIXME Try to use a QHash (this requires to define qHash(docstring))
	mutable std::map<docstring, int> strwidth_cache_;

	struct AscendDescend {
		int ascent;
		int descent;
	};
	/// Cache of char ascends and descends
	mutable QHash<char_type, AscendDescend> metrics_cache_;
	/// fill in \c metrics_cache_ at specified value.
	AscendDescend const fillMetricsCache(char_type) const;

	/// Cache of char right bearings
	mutable QHash<char_type, int> rbearing_cache_;
};

} // namespace frontend
} // namespace lyx

#endif // GUI_FONT_METRICS_H
