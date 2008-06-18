// -*- C++ -*-
/**
 * \file InsetBibitem.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_BIBITEM_H
#define INSET_BIBITEM_H

#include "BiblioInfo.h"
#include "InsetCommand.h"


namespace lyx {

/////////////////////////////////////////////////////////////////////////
//
// InsetBibItem
//
/////////////////////////////////////////////////////////////////////////

/// Used to insert bibitem's information (key and label)

//  Must be automatically inserted as the first object in a
//  bibliography paragraph.
class InsetBibitem : public InsetCommand
{
public:
	///
	InsetBibitem(Buffer const &, InsetCommandParams const &);
	///
	virtual ~InsetBibitem();
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "bibitem"; }
	///
	static bool isCompatibleCommand(std::string const & s) 
		{ return s == "bibitem"; }
private:
	/// verify label and update references.
	/// Overloaded from Inset::initView.
	void initView();
	///
	bool isLabeled() const { return true; }
	///
	void read(Lexer & lex);
	///
	docstring screenLabel() const;
	///
	EDITABLE editable() const { return IS_EDITABLE; }
	///
	InsetCode lyxCode() const { return BIBITEM_CODE; }
	///
	docstring bibLabel() const;
	///
	int plaintext(odocstream &, OutputParams const &) const;
	///
	virtual void fillWithBibKeys(BiblioInfo &, InsetIterator const &) const;
	/// Update the counter of this inset
	virtual void updateLabels(ParIterator const &);
	///
	void updateCommand(docstring const & new_key, bool dummy = false);
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetBibitem(*this); }

	friend docstring bibitemWidest(Buffer const & buffer);
	/// The label that is set by updateLabels
	docstring autolabel_;
	///
	static int key_counter;
};


/// Return the widest label in the Bibliography.
docstring bibitemWidest(Buffer const &);

} // namespace lyx

#endif // INSET_BIBITEM_H
