// -*- C++ -*-
/**
 * \file GuiMath.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Angus Leeming
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef GUIMATH_H
#define GUIMATH_H

#include "GuiDialog.h"
#include "FuncCode.h"
#include "FontEnums.h"

#include <map>
#include <string>


namespace lyx {
namespace frontend {

struct MathSymbol {
	MathSymbol(char_type uc = '?', unsigned char fc = 0,
		FontFamily ff = SYMBOL_FAMILY)
		: unicode(uc), fontcode(fc), fontfamily(ff)
	{}
	char_type unicode;
	unsigned char fontcode;
	FontFamily fontfamily;
};


class GuiMath : public GuiDialog
{
public:
	GuiMath(GuiView & lv, QString const & name, QString const & title);

	/// Nothing to initialise in this case.
	bool initialiseParams(std::string const &) { return true; }
	void clearParams() {}
	void dispatchParams() {}
	bool isBufferDependent() const { return true; }

	/// dispatch an LFUN
	void dispatchFunc(FuncCode action,
		std::string const & arg = std::string()) const;
	/// Insert a matrix
	void dispatchMatrix(std::string const & str) const;
	/// Insert a variable size delimiter
	void dispatchDelim(std::string const & str) const;
	/// Insert a big delimiter
	void dispatchBigDelim(std::string const & str) const;
	/** A request to the kernel to launch a dialog.
	 *  \param name the dialog identifier.
	 */
	void showDialog(std::string const & name) const;

	/// \return the math unicode symbol associated to a TeX name.
	MathSymbol const & mathSymbol(std::string tex_name) const;
	/// \return the TeX name associated to a math unicode symbol.
	std::string const & texName(char_type math_symbol) const;

private:
	/// TeX-name / Math-symbol map.
	std::map<std::string, MathSymbol> math_symbols_;
	/// Math-symbol / TeX-name map.
	/// This one is for fast search, it contains the same data as
	/// \c math_symbols_.
	std::map<char_type, std::string> tex_names_;
};


} // namespace frontend
} // namespace lyx

#endif // GUIMATH_H
