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

#include "support/Cache.h"
#include "support/docstring.h"

#include <QFont>
#include <QFontMetrics>
#include <QHash>
#include <QTextLayout>

#include <memory>

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
	virtual int em() const;
	virtual int xHeight() const;
	virtual int lineWidth() const;
	virtual int underlinePos() const;
	virtual int strikeoutPos() const;
	virtual int width(char_type c) const;
	virtual int ascent(char_type c) const;
	virtual int descent(char_type c) const;
	virtual int lbearing(char_type c) const;
	virtual int rbearing(char_type c) const;
	virtual int width(docstring const & s) const;
	virtual int signedWidth(docstring const & s) const;
	virtual int pos2x(docstring const & s, int pos, bool rtl, double ws) const;
	virtual int x2pos(docstring const & s, int & x, bool rtl, double ws) const;
	virtual bool breakAt(docstring & s, int & x, bool rtl, bool force) const;
	virtual Dimension const dimension(char_type c) const;

	virtual void rectText(docstring const & str,
		int & width,
		int & ascent,
		int & descent) const;
	virtual void buttonText(docstring const & str,
		const int offset,
		int & width,
		int & ascent,
		int & descent) const;

	int countExpanders(docstring const & str) const;
	///
	int width(QString const & str) const;

	/// Return a pointer to a cached QTextLayout object
	std::shared_ptr<QTextLayout const>
	getTextLayout(docstring const & s, bool const rtl,
	              double const wordspacing) const;

private:

	std::pair<int, int> breakAt_helper(docstring const & s, int const x,
	                                   bool const rtl, bool const force) const;

	/// The font
	QFont font_;

	/// Metrics on the font
	QFontMetrics metrics_;

	/// Cache of char widths
	mutable QHash<char_type, int> width_cache_;
	/// Cache of string widths
	mutable Cache<docstring, int> strwidth_cache_;
	/// Cache for breakAt
	mutable Cache<docstring, std::pair<int, int>> breakat_cache_;
	/// Cache for QTextLayout
	mutable Cache<docstring, std::shared_ptr<QTextLayout>> qtextlayout_cache_;

	struct AscendDescend {
		int ascent;
		int descent;
	};
	/// Cache of char ascends and descends
	mutable QHash<char_type, AscendDescend> metrics_cache_;
	/// fill in \c metrics_cache_ at specified value.
	AscendDescend const fillMetricsCache(char_type) const;

	/// Cache of char left bearings
	mutable QHash<char_type, int> lbearing_cache_;
	/// Cache of char right bearings
	mutable QHash<char_type, int> rbearing_cache_;

};

} // namespace frontend
} // namespace lyx

#endif // GUI_FONT_METRICS_H
