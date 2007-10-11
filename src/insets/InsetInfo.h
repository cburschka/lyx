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

namespace lyx {

/** Used to insert index labels
  */
class InsetInfo : public InsetText {
public:
	enum info_type {
		UNKNOWN_INFO,   // Invalid type
		SHORTCUT_INFO,  // Keyboard shortcut
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
	Inset::Code lyxCode() const { return Inset::INFO_CODE; }
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
