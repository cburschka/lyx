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

#include "ColorSet.h"
#include "Length.h"
#include "support/qstring_helpers.h"
#include "support/filetools.h"
#include "qt_i18n.h"

#include <QHeaderView>

class QComboBox;
class QLineEdit;
class QCheckBox;
class QString;
class QWidget;
template <class T> class QList;

namespace lyx {

namespace support { class FileName; }

class BufferParams;
 
namespace frontend {

class LengthCombo;

/// method to get a Length from widgets (LengthCombo)
std::string widgetsToLength(QLineEdit const * input, LengthCombo const * combo);
/// method to get a Length from widgets (QComboBox)
Length widgetsToLength(QLineEdit const * input, QComboBox const * combo);

/// method to set widgets from a Length
//FIXME Remove default_unit argument for the first form. FIXME Change
// all the code to remove default_unit argument when equal to the
// default.
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
		     Length const & len, 
		     Length::UNIT default_unit = Length::defaultUnit());
/// method to set widgets from a string
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
		     std::string const & len, 
		     Length::UNIT default_unit = Length::defaultUnit());
/// method to set widgets from a docstring
void lengthToWidgets(QLineEdit * input, LengthCombo * combo,
docstring const & len, Length::UNIT default_unit);

/// method to get a double value from a localized widget (QLineEdit)
double widgetToDouble(QLineEdit const * input);
/// method to get a double value from a localized widget (QLineEdit)
std::string widgetToDoubleStr(QLineEdit const * input);
/// method to set a (localized) double value in a widget (QLineEdit)
void doubleToWidget(QLineEdit * input, double const & value,
	char f = 'g', int prec = 6);
/// method to set a (localized) double value in a widget (QLineEdit)
void doubleToWidget(QLineEdit * input, std::string const & value,
	char f = 'g', int prec = 6);
/**
 * method to format localized floating point numbers without
 * ever using scientific notation
 */
QString formatLocFPNumber(double d);

/// Method to sort colors by GUI name in combo widgets
bool ColorSorter(ColorCode lhs, ColorCode rhs);

/// colors a widget red if invalid
void setValid(QWidget * widget, bool valid);

/// Qt5 changed setSectionMode to setSectionResizeMode
/// These wrappers work for Qt4 and Qt5
void setSectionResizeMode(QHeaderView * view,
    int logicalIndex, QHeaderView::ResizeMode mode);
void setSectionResizeMode(QHeaderView * view,
	QHeaderView::ResizeMode mode);

} // namespace frontend


/**
 * qt_ - i18nize string and convert to QString
 *
 * Use this in qt4/ instead of _()
 */
QString const qt_(std::string const & str);
QString const qt_(QString const & qstr);


///
support::FileName libFileSearch(QString const & dir, QString const & name,
				QString const & ext = QString(),
				support::search_mode mode = support::must_exist);

///
support::FileName imageLibFileSearch(QString & dir, QString const & name,
				QString const & ext = QString(),
				support::search_mode mode = support::must_exist);

/** Wrappers around browseFile which try to provide a filename
	relative to relpath.

\param title: title for dialog
	
\param filters: *.ps, etc

\param save: whether to save dialog info (current path, etc) for next use.

The \param labelN and \param dirN arguments provide for extra buttons 
in the dialog (e.g., "Templates" and a path to that directory).

The difference between the functions concerns when we think we have a 
relative path. 

In \c browseRelToParent, we return a relative path only if it IS NOT of 
	the form "../../foo.txt".

In \c browseRelToSub, we return a relative path only if it IS of the
 form "../../foo.txt".
*/
QString browseRelToParent(QString const & filename,
	QString const & relpath,
	QString const & title,
	QStringList const & filters,
	bool save = false,
	QString const & label1 = QString(),
	QString const & dir1 = QString(),
	QString const & label2 = QString(),
	QString const & dir2 = QString());

QString browseRelToSub(QString const & filename,
	QString const & relpath,
	QString const & title,
	QStringList const & filters,
	bool save = false,
	QString const & label1 = QString(),
	QString const & dir1 = QString(),
	QString const & label2 = QString(),
	QString const & dir2 = QString());

/** Build filelists of all availabe bst/cls/sty-files. Done through
*  kpsewhich and an external script, saved in *Files.lst.
*  \param arg: cls, sty, bst, or bib, as required by TeXFiles.py.
*         Can be a list of these, too.
*/
void rescanTexStyles(std::string const & arg = empty_string());

/** Fill \c contents from one of the three texfiles.
 *  Each entry in the file list is returned as a name_with_path
 */
QStringList texFileList(QString const & filename);

/// Convert internal line endings to line endings as expected by the OS
QString const externalLineEnding(docstring const & str);

/// Convert line endings in any formnat to internal line endings
docstring const internalLineEnding(QString const & str);

// wrapper around the docstring versions
QString internalPath(QString const &);
QString onlyFileName(QString const & str);
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

/// \return the display string associated with given type and buffer
/// parameter.
QString guiName(std::string const & type, BufferParams const & bp);

/// Format \param text for display as a ToolTip, breaking at lines of \param
/// width ems. Note: this function is expensive. Better call it in a delayed
/// manner, i.e. not to fill in a model (see for instance the function
/// ToolTipFormatter::eventFilter).
///
/// When is it called automatically? Whenever the tooltip is not already rich
/// text beginning with <html>, and is defined by the following functions:
///  - QWidget::setToolTip(),
///  - QAbstractItemModel::setData(..., Qt::ToolTipRole),
///  - Inset::toolTip()
///
/// In other words, tooltips can use Qt html, and the tooltip will still be
/// correctly broken. Moreover, it is possible to specify an entirely custom
/// tooltip (not subject to automatic formatting) by giving it in its entirety,
/// i.e. starting with <html>.
QString formatToolTip(QString text, int width = 30);


#if QT_VERSION < 0x050300
// Very partial implementation of QSignalBlocker for archaic qt versions.
class QSignalBlocker {
public:
	explicit QSignalBlocker(QObject * o)
		: obj(o), init_state(obj && obj->blockSignals(true)) {}

	~QSignalBlocker() {
		if (obj)
			obj->blockSignals(init_state);
	}
private:
	QObject * obj;
	bool init_state;
};
#endif


} // namespace lyx

#endif // QTHELPERS_H
