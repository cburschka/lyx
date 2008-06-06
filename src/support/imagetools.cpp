/**
 * \file imagetools.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Abdelrazak Younes
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/debug.h"
#include "support/imagetools.h"
#include "support/lstrings.h"

#include <QByteArray>
#include <QImageReader>
#include <QList>

using namespace std;

namespace lyx {
namespace support {

/// Return the list of loadable formats.
vector<string> loadableImageFormats()
{
	vector<string> fmts;

	QList<QByteArray> qt_formats = QImageReader::supportedImageFormats();

	LYXERR(Debug::GRAPHICS,
		"\nThe image loader can load the following directly:\n");

	if (qt_formats.empty())
		LYXERR(Debug::GRAPHICS, "\nQt4 Problem: No Format available!");

	for (QList<QByteArray>::const_iterator it = qt_formats.begin(); it != qt_formats.end(); ++it) {

		LYXERR(Debug::GRAPHICS, (const char *) *it << ", ");

		string ext = ascii_lowercase((const char *) *it);
		// special case
		if (ext == "jpeg")
			ext = "jpg";
		fmts.push_back(ext);
	}

	return fmts;
}


} // namespace support
} // namespace lyx
