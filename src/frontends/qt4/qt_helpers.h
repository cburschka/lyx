// -*- C++ -*-
/**
 * \file qt_helpers.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Dekel Tsur
 * \author Richard Heck
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef QTHELPERS_H
#define QTHELPERS_H

#include "Length.h"
#include "support/qstring_helpers.h"
#include "qt_i18n.h"

#include <QString>

class QComboBox;
class QLineEdit;
class QCheckBox;
class QString;
class QWidget;
template <class T> class QList;

class LengthCombo;

namespace lyx {

namespace support { class FileName; }

class LyXErr;

LyXErr & operator<<(LyXErr &, QString const &);

namespace frontend {

/// method to get a Length from widgets (LengthCombo)
std::string widgetsToLength(QLineEdit const * input, LengthCombo const * combo);
/// method to get a Length from widgets (QComboBox)
Length widgetsToLength(QLineEdit const * input, QComboBox const * combo);

//FIXME It would be nice if defaultUnit were a default argument
/// method to set widgets from a Length
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
Length const & len, Length::UNIT default_unit);
/// method to set widgets from a string
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
std::string const & len, Length::UNIT default_unit);
/// method to set widgets from a Length with optional "auto" if zero
void lengthAutoToWidgets(QLineEdit * input, LengthCombo * combo,
Length const & len, Length::UNIT defaultUnit);

/// colors a widget red if invalid
void setValid(QWidget * widget, bool valid);

} // namespace frontend


/**
 * qt_ - i18nize string and convert to QString
 *
 * Use this in qt4/ instead of _()
 */
QString const qt_(std::string const & str);

///
typedef std::pair<QString, QString> LanguagePair;

///
QList<LanguagePair> languageData();

support::FileName libFileSearch(QString const & dir, QString const & name,
				QString const & ext = QString());

/** Wrapper around browseFile which tries to provide a filename
	relative to relpath.  If the relative path is of the form "foo.txt"
	or "bar/foo.txt", then it is returned as relative. OTOH, if it is
	of the form "../baz/foo.txt", an absolute path is returned. This is
	intended to be useful for insets which encapsulate files/
*/
QString browseRelFile(QString const & filename,
	QString const & refpath,
	QString const & title,
	QStringList const & filters,
	bool save = false,
	QString const & label1 = QString(),
	QString const & dir1 = QString(),
	QString const & label2 = QString(),
	QString const & dir2 = QString());


/** Build filelists of all availabe bst/cls/sty-files. Done through
*  kpsewhich and an external script, saved in *Files.lst.
*/
void rescanTexStyles();

/** Fill \c contents from one of the three texfiles.
 *  Each entry in the file list is returned as a name_with_path
 */
QStringList texFileList(QString const & filename);

// wrapper around the docstring versions
QString internalPath(QString const &);
QString onlyFilename(QString const & str);
QString onlyPath(QString const & str);
QStringList fileFilters(QString const & description);

QString changeExtension(QString const & oldname, QString const & extension);

/// Remove the extension from \p name
QString removeExtension(QString const & name);

/** Add the extension \p ext to \p name.
 Use this instead of changeExtension if you know that \p name is without
 extension, because changeExtension would wrongly interpret \p name if it
 contains a dot.
 */
QString addExtension(QString const & name, QString const & extension);

/// Return the extension of the file (not including the .)
QString getExtension(QString const & name);
QString makeAbsPath(QString const & relpath, QString const & base);
QString changeExtension(QString const & oldname, QString const & ext);

} // namespace lyx

#endif // QTHELPERS_H
