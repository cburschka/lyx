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

#include "InsetText.h"
#include "RenderButton.h"
#include "Cursor.h"

#include "support/gettext.h"
#include "support/Translator.h"

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
or "buffer". Arguments and outputs vary by type.

shortcuts: argument of this type of InsetInfo is the name of the LFUN such as
    "math-insert \alpha". The syntax is the same as what is used in the bind
    and ui files. The output of this inset is all shortcuts for this LFUN
    separated by comma.

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

icon: argument is the name of the LFUN such as "paste". The syntax is the same
    as what is used in the bind and ui files. The output is the icon use in
    the toolbar for this LFUN.

buffer: argument can be one of "name", "path", "class". This inset output the
    filename, path, and textclass of this buffer.

There is currently no GUI, no menu entry for this inset. A user can define a
shortcut for "info-insert" (e.g. C-S-I), and

1. input the type and argument of this inset, e.g. "menu paste", in
   the work area.
2. select the text and run "info-insert" (e.g. press C-S-I).

An alternative method is to enter command "info-insert type arg" in
the command buffer (view->Toolbar->Command Buffer).

*/

namespace lyx {

/** Used to insert index labels
  */
class InsetInfo : public InsetText {
public:
	enum info_type {
		UNKNOWN_INFO,   // Invalid type
		SHORTCUTS_INFO, // Keyboard shortcuts
		SHORTCUT_INFO,  // Keyboard shortcut
		LYXRC_INFO,     // RC entry
		PACKAGE_INFO,   // Availability of package
		TEXTCLASS_INFO, // Availability of textclass
		MENU_INFO,      // Which menu item is used for certain function
		ICON_INFO,      // which toolbar icon is used for certain function
		BUFFER_INFO,    // Buffer related information
	};

	///
	InsetInfo(Buffer const & buf, std::string const & info = std::string());
	///
	Inset * editXY(Cursor & cur, int x, int y);
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void read(Lexer & lex);
	///
	void write(std::ostream & os) const;
	///
	std::string infoType() const;
	///
	std::string infoName() const { return name_; }
	///
	bool validate(docstring const & argument) const;
	///
	bool showInsetDialog(BufferView * bv) const;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	InsetCode lyxCode() const { return INFO_CODE; }
	///
	void setInfo(std::string const & info);
	/// update info_ and text
	void updateInfo();
	///
	bool setMouseHover(bool mouse_hover);
	///
	docstring toolTip(BufferView const & bv, int x, int y) const;
	///
	docstring contextMenu(BufferView const &, int, int) const;

private:
	/// The translator between the information type enum and corresponding string.
	Translator<info_type, std::string> const & nameTranslator() const;
	///
	virtual Inset * clone() const { return new InsetInfo(*this); }
	///
	void error(std::string const & err);
	///
	void setText(docstring const & str);
	///
	info_type type_;
	///
	std::string name_;
	///
	bool mouse_hover_;
};



} // namespace lyx

#endif
