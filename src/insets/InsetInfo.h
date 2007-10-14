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
#include "gettext.h"
#include "Cursor.h"
#include "support/Translator.h"

/* InsetInfo displays shortcuts, lyxrc, package and textclass 
availability and menu information in a non-editable boxed InsetText.

Output of such as inset may vary from system to system, depending
on LyX and LaTeX configurations. Two LyX help files, LaTeXConfig.lyx
and Shortcuts.lyx make heavy use of this inset. The former uses it
to display the availability of packages and textclasses, the latter
uses it to display currently used shortcuts.

This inset has two arguments: the type and argument of the information.
The screen and latex output is the content of the information. An 
InsetInfo can have type "shortcut", "lyxrc", "package", "textclass", or
"menu". Arguments and outputs vary by type.

shortcut: name of the LFUN such as "math-insert \alpha". The syntax
    is the same as what is used in the bind and ui files.
    The output of this inset is all shortcuts separated by comma.

lyxrc: name of the rc entry such as "bind_file". Look in src/LyXRC.h
    for available entries. The output is the content of this RC
    entry.

package: name of a latex package such as "listings". The output is
    "yes" or "no".

textclass: name of a textclass such as "article". The output is "yes"
    or "no".

menu: name of the LFUN such as "paste". The syntax is the same as
    what is used in the bind and ui files. The output is the
    menu item that trigger this LFUN. For example, "File > Paste".
    '>' is actually \lyxarrow (an InsetSpecialChar).


Because this inset is intended to be used only by document maintainers,
there is no GUI, no menu entry for this inset. A user can define a 
shortcut for "info-insert" (e.g. C-S-I), and

1. input the type and argument of this inset, e.g. "menu paste", in 
   the work area.
2. select the text and run "info-insert" (e.g. press C-S-I).

An alternative method is to enter command "info-insert type args" in 
the command buffer (view->Toolbar->Command Buffer).

*/

namespace lyx {

/** Used to insert index labels
  */
class InsetInfo : public InsetText {
public:
	enum info_type {
		UNKNOWN_INFO,   // Invalid type
		SHORTCUT_INFO,  // Keyboard shortcut
		LYXRC_INFO,     // RC entry
		PACKAGE_INFO,   // Availability of package
		TEXTCLASS_INFO, // Availability of textclass
		MENU_INFO,      // Which menu item is used for certain function
	};

	///
	InsetInfo(BufferParams const & bp, std::string const & info = std::string());
	///
	Inset * editXY(Cursor & cur, int x, int y);
	///
	EDITABLE editable() const { return NOT_EDITABLE; }
	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void read(Buffer const &, Lexer & lex);
	///
	void write(Buffer const & buf, std::ostream & os) const;
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	InsetCode lyxCode() const { return INFO_CODE; }
	///
	void setInfo(std::string const & info);
	///
	bool setMouseHover(bool mouse_hover);

private:
	/// The translator between the information type enum and corresponding string.
	Translator<info_type, std::string> const & nameTranslator() const;
	/// update info_ and text
	void updateInfo();
	///
	virtual Inset * clone() const { return new InsetInfo(*this); }
	///
	info_type type_;
	///
	std::string name_;
	/// store the buffer parameter
	BufferParams const & bp_;
	///
	bool mouse_hover_;
};



} // namespace lyx

#endif
