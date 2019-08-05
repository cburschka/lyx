// -*- C++ -*-
/**
 * \file InsetInfo.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Bo Peng
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_INFO_H
#define INSET_INFO_H

#include "InsetCollapsible.h"
#include <QDate>

namespace lyx {

class Cursor;

/* InsetInfo displays shortcuts, lyxrc, package and textclass
availability and menu information in a non-editable boxed InsetText.

Output of such an inset may vary from system to system, depending
on LyX and LaTeX configurations. Two LyX help files, LaTeXConfig.lyx
and Shortcuts.lyx make heavy use of this inset. The former uses it
to display the availability of packages and textclasses, the latter
uses it to display currently used shortcuts.

This inset has two arguments: the type and argument of the information. The
screen and latex output is the content of the information. An InsetInfo can
have type "shortcuts", "shortcut", "lyxrc", "package", "textclass", "menu",
"buffer" or "vcs". Arguments and outputs vary by type.

date: argument of this type of InsetInfo is either a fixed date type of
    "long" (long localized date, with weekday, as provided by QLocale),
    "short" (short localized date, with two-digit year, as provided by QLocale),
    "loclong" (long localized date, without weekday, defined in languages),
    "locmedium" (medium localized date, defined in languages),
    "locshort" (short localized date, with four-digit year, defined in languages),
    "ISO" (ISO-conforming date)
    or a custom date using the QDate syntax.
    The output is a localized formatted (current) date.

moddate: Same as date.
    The output is a localized formatted date of last file modification (saving).

fixdate: Same as date. A fixed date (in ISO format) is prepended to the argument,
    delimited by '@'.
    The output is a localized formatted fix date.

time: argument of this type of InsetInfo is either a fixed time type of
    "long" (long localized time, as provided by QLocale),
    "short" (short localized time, as provided by QLocale),
    "ISO" (ISO-conforming time)
    or a custom date using the QTime syntax.
    The output is a localized formatted (current) time.

modtime: Same as time.
    The output is a localized formatted time of last file modification (saving).

fixtime: Same as time. A fixed time (in ISO format) is prepended to the argument,
    delimited by '@'.
    The output is a localized formatted fix time.

shortcuts: argument is the name of the LFUN such as "math-insert \alpha".
    The syntax is the same as what is used in the bind and ui files.
    The output of this inset is all shortcuts for this LFUN separated by comma.

shortcut: the same as shortcuts, but only output the last shortcut.

lyxrc: argument is the name of the rc entry such as "bind_file". Look in
    src/LyXRC.h for available entries. The output is the content of this RC
    entry.

package: argument is the name of a latex package such as "listings". The
    output is "yes" or "no", indicating the availability of this package.

textclass: argument is the name a textclass such as "article". The output is
    "yes" or "no", indicating the availability of this textclass.

menu: argument is the name of the LFUN such as "paste". The syntax is the same
    as what is used in the bind and ui files. The output is the menu item that
    triggers this LFUN. For example, "File > Paste", where '>' is actually
    \lyxarrow (an InsetSpecialChar).

l7n: argument is an English string that is marked for localization. The output
    is the localization of that string in the current GUI language (if available).
    Trailing colons are stripped, accelerators removed.
    This is used to refer to GUI items in the docs.

icon: argument is the name of the LFUN such as "paste". The syntax is the same
    as what is used in the bind and ui files. The output is the icon use in
    the toolbar for this LFUN. Alternatively, argument can be the icon path
    without extension specified with respect to the images directory.

buffer: argument can be one of "name", "name-noext", "path", "class". This inset output the
    filename (with extension), filename (without extension), path, and textclass of this buffer.

lyxinfo: argument must be "version" or "layoutformat". This outputs information
    about the version of LyX currently in use or the current LyX layout format, respectively.

vcs: argument can be one of "revision", "tree-revision", "author", "time", "date".
    This insets outputs revision control information, if available.

There is currently no GUI, no menu entry for this inset. A user can define a
shortcut for "info-insert" (e.g. C-S-I), and

1. input the type and argument of this inset, e.g. "menu paste", in
   the work area.
2. select the text and run "info-insert" (e.g. press C-S-I).

An alternative method is to enter command "info-insert type arg" in
the command buffer (view->Toolbar->Command Buffer).

*/

class InsetInfoParams {
public:
	enum info_type {
		DATE_INFO,       // Current date
		MODDATE_INFO,    // Date of last modification
		FIXDATE_INFO,    // Fix date
		TIME_INFO,       // Current time
		MODTIME_INFO,    // Time of last modification
		FIXTIME_INFO,    // Fix time
		BUFFER_INFO,     // Buffer related information
		VCS_INFO,        // Version control information
		PACKAGE_INFO,    // Availability of package
		TEXTCLASS_INFO,  // Availability of textclass
		SHORTCUTS_INFO,  // Keyboard shortcuts
		SHORTCUT_INFO,   // Keyboard shortcut
		LYXRC_INFO,      // RC entry
		MENU_INFO,       // Which menu item is used for certain function
		ICON_INFO,       // which toolbar icon is used for certain function
		LYX_INFO,        // LyX version information
		L7N_INFO,        // Localized string
		UNKNOWN_INFO,    // Invalid type
	};
	///
	docstring getDate(std::string const, QDate const date = QDate::currentDate()) const;
	///
	docstring getTime(std::string const, QTime const time = QTime::currentTime()) const;
	///
	std::vector<std::pair<std::string,docstring>> getArguments(Buffer const * buf,
								   std::string const &) const;
	///
	bool validateArgument(Buffer const * buf, docstring const & argument,
			      bool const usedefault = false) const;
	///
	info_type type;
	///
	std::string infoType() const;
	///
	std::string name;
	///
	Language const * lang;
	///
	bool force_ltr;
};

///
extern InsetInfoParams infoparams;

class InsetInfo : public InsetCollapsible {
public:
	///
	InsetInfo(Buffer * buf, std::string const & info = std::string());
	///
	InsetCode lyxCode() const { return INFO_CODE; }
	///
	docstring layoutName() const;
	///
	Inset * editXY(Cursor & cur, int x, int y);
	/** FIXME: we would like to do that, but then InsetText::updateBuffer breaks
	 * on info insets. Do we need to run this method on InsetInfo contents?
	 * Having a InsetInfo that hides an InsetText is really annoying, actually.
	 */
	///bool isActive() const { return false; }
	///
	bool editable() const { return false; }
	///
	bool hasSettings() const { return true; }
	///
	void read(Lexer & lex);
	///
	void write(std::ostream & os) const;
	///
	bool validateModifyArgument(docstring const & argument) const {
		return params_.validateArgument(&buffer(), argument); }
	///
	bool showInsetDialog(BufferView * bv) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	/// Force inset into LTR environment if surroundings are RTL
	bool forceLTR(OutputParams const &) const;
	///
	void setInfo(std::string const & info);
	///
	void updateBuffer(ParIterator const & it, UpdateType utype);
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	std::string contextMenu(BufferView const &, int, int) const;
	///
	std::string contextMenuName() const;
	/// should paragraph indendation be omitted in any case?
	bool neverIndent() const { return true; }
	///
	InsetInfoParams params() const { return params_; }

private:
	///
	virtual Inset * clone() const { return new InsetInfo(*this); }
	///
	void error(docstring const & err, Language const *);
	///
	void info(docstring const & err, Language const *);
	///
	void setText(docstring const & str, Language const *);
	// make sure that the other version of setText is still available.
	using InsetCollapsible::setText;
	///
	bool initialized_;
	///
	InsetInfoParams params_;
	///
	friend class InsetInfoParams;
};


} // namespace lyx

#endif
