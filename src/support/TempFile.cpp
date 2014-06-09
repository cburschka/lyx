/**
 * \file TempFile.cpp
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#include <config.h>

#include "support/TempFile.h"

#include "support/debug.h"
#include "support/FileName.h"
#include "support/Package.h"
#include "support/qstring_helpers.h"

#include <QFileInfo>
#include <QDir>
#include <QTemporaryFile>

using namespace std;

namespace lyx {
namespace support {

struct TempFile::Private
{
	///
	Private(QString const & mask) : f(mask)
	{
		LYXERR(Debug::FILES, "Temporary file in " << fromqstr(mask));
		if (f.open())
			LYXERR(Debug::FILES, "Temporary file `"
			       << fromqstr(f.fileName()) << "' created.");
		else
			LYXERR(Debug::FILES, "Unable to create temporary file with following template: "
			       << f.fileTemplate());
	}

	///
	QTemporaryFile f;
};


TempFile::TempFile(FileName const & temp_dir, string const & mask)
{
	QFileInfo tmp_fi(QDir(toqstr(temp_dir.absoluteFilePath())), toqstr(mask));
	d = new Private(tmp_fi.absoluteFilePath());
}


TempFile::TempFile(string const & mask)
{
	QFileInfo tmp_fi(QDir(toqstr(package().temp_dir().absoluteFilePath())), toqstr(mask));
	d = new Private(tmp_fi.absoluteFilePath());
}


TempFile::~TempFile()
{
	delete d;
}


FileName TempFile::name() const
{
	QString const n = d->f.fileName();
	if (n.isNull())
		return FileName();
	return FileName(fromqstr(n));
}


void TempFile::setAutoRemove(bool autoremove)
{
	d->f.setAutoRemove(autoremove);
}

} // namespace support
} // namespace lyx
