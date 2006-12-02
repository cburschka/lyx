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
#include <QHash>

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
#ifndef USE_LYX_FONTCACHE
	virtual int width(char_type c) const {
		return metrics_.width(QChar(static_cast<short int>(c)));
	}
#else
	virtual int width(char_type c) const;
#endif
	virtual int ascent(char_type c) const;
	virtual int descent(char_type c) const;
	virtual int lbearing(char_type c) const;
	virtual int rbearing(char_type c) const;
	virtual int width(char_type const * s, size_t n) const;
	virtual int signedWidth(docstring const & s) const;
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
	int smallcapsWidth(QString const & s) const;

	/// Metrics on the font
	QFontMetrics metrics_;
	QFontMetrics smallcaps_metrics_;

	bool smallcaps_shape_;

#ifdef USE_LYX_FONTCACHE

	/// Cache of char widths
	mutable QHash<char_type, int> width_cache_;

	struct AscendDescend {
		short int ascent;
		short int descent;
	};
	/// Cache of char ascends and descends
	mutable QHash<char_type, AscendDescend> metrics_cache_;
	/// fill in \c metrics_cache_ at specified value.
	void fillMetricsCache(char_type) const;

#endif // USE_LYX_FONTCACHE
};

} // namespace frontend
} // namespace lyx

#endif // QT4_FONT_METRICS_H
