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

struct BreakAtKey
{
	bool operator==(BreakAtKey const & key) const {
		return key.s == s && key.x == x && key.rtl == rtl && key.force == force;
	}

	docstring s;
	int x;
	bool rtl;
	bool force;
};

struct TextLayoutKey
{
	bool operator==(TextLayoutKey const & key) const {
		return key.s == s && key.rtl == rtl && key.ws == ws;
	}

	docstring s;
	bool rtl;
	double ws;
};


class GuiFontMetrics : public FontMetrics
{
public:
	GuiFontMetrics(QFont const & font);

	virtual ~GuiFontMetrics() {}

	int maxAscent() const override;
	int maxDescent() const override;
	Dimension const defaultDimension() const override;
	int em() const override;
	int xHeight() const override;
	int lineWidth() const override;
	int underlinePos() const override;
	int strikeoutPos() const override;
	bool italic() const override;
	double italicSlope() const override;
	int width(char_type c) const override;
	int ascent(char_type c) const override;
	int descent(char_type c) const override;
	int lbearing(char_type c) const override;
	int rbearing(char_type c) const override;
	int width(docstring const & s) const override;
	int signedWidth(docstring const & s) const override;
	int pos2x(docstring const & s, int pos, bool rtl, double ws) const override;
	int x2pos(docstring const & s, int & x, bool rtl, double ws) const override;
	int breakAt(docstring const & s, int & x, bool rtl, bool force) const override;
	Dimension const dimension(char_type c) const override;

	void rectText(docstring const & str,
		int & width,
		int & ascent,
		int & descent) const override;
	void buttonText(docstring const & str,
		const int offset,
		int & width,
		int & ascent,
		int & descent) const override;

	int countExpanders(docstring const & str) const override;
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

	/// Slope of italic font
	double slope_;

	/// Cache of char widths
	mutable QHash<char_type, int> width_cache_;
	/// Cache of string widths
	mutable Cache<docstring, int> strwidth_cache_;
	/// Cache for breakAt
	mutable Cache<BreakAtKey, std::pair<int, int>> breakat_cache_;
	/// Cache for QTextLayout
	mutable Cache<TextLayoutKey, std::shared_ptr<QTextLayout>> qtextlayout_cache_;

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
