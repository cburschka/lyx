// -*- C++ -*-
/**
 * \file FontMetrics.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QT4_FONT_METRICS_H
#define QT4_FONT_METRICS_H

#include "frontends/FontMetrics.h"

#include "support/docstring.h"

#include <QFontMetrics>

// Starting with version 3.1.0, Qt/X11 does its own caching of
// character width, so it is not necessary to provide ours.
#if defined(Q_WS_MACX) || defined(Q_WS_WIN32)
#define USE_LYX_FONTCACHE
#endif

namespace lyx {
namespace frontend {

class GuiFontMetrics: public FontMetrics
{
public:

	GuiFontMetrics(QFont const & font);
	GuiFontMetrics(QFont const & font, QFont const & smallcaps_font);

	virtual ~GuiFontMetrics() {}

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
	///
	int width(QString const & str) const;

private:
	int smallcapsWidth(QString const & s) const;

	/// Metrics on the font
	QFontMetrics metrics_;
	QFontMetrics smallcaps_metrics_;

	bool smallcaps_shape_;

#ifndef USE_LYX_FONTCACHE
	/// Return pixel width for the given unicode char
	int width(unsigned short val) const { return metrics_.width(QChar(val)); }

#else
	/// Return pixel width for the given unicode char
	int width(unsigned short val) const;

	/// Cache of char widths
	mutable int widthcache_[65536];
#endif // USE_LYX_FONTCACHE
};

} // namespace frontend
} // namespace lyx

#endif // QT4_FONT_METRICS_H
