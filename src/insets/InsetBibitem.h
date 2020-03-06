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


#include "InsetCommand.h"


namespace lyx {

class BiblioInfo;

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
	InsetBibitem(Buffer *, InsetCommandParams const &);
	///
	~InsetBibitem();

	///
	void updateCommand(docstring const & new_key, bool dummy = false);

	/// \name Public functions inherited from Inset class
	//@{
	///
	InsetCode lyxCode() const { return BIBITEM_CODE; }
	///
	bool hasSettings() const { return true; }
	/// \copydoc Inset::initView()
	/// verify label and update references.
	void initView();
	///
	bool isLabeled() const { return true; }
	///
	void read(Lexer & lex);
	///
	int plaintext(odocstringstream &, OutputParams const &,
	              size_t max_length = INT_MAX) const;
	///
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	void collectBibKeys(InsetIterator const &, support::FileNameList &) const;
	/// update the counter of this inset
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false);
	///@}

	/// \name Static public methods obligated for InsetCommand derived classes
	//@{
	///
	static ParamInfo const & findInfo(std::string const &);
	///
	static std::string defaultCommand() { return "bibitem"; }
	///
	static bool isCompatibleCommand(std::string const & s)
		{ return s == "bibitem"; }
	///@}

private:
	///
	docstring bibLabel() const;

	/// \name Private functions inherited from Inset class
	//@{
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd);
	///
	Inset * clone() const { return new InsetBibitem(*this); }
	/// Is the content of this inset part of the immediate (visible) text sequence?
	bool isPartOfTextSequence() const { return false; }
	///@}

	/// \name Private functions inherited from InsetCommand class
	//@{
	///
	docstring screenLabel() const;
	//@}

	friend docstring bibitemWidest(Buffer const & buffer, OutputParams const &);

	/// The label that is set by updateBuffer
	docstring autolabel_;
	///
	static int key_counter;
};


/// Return the widest label in the Bibliography.
docstring bibitemWidest(Buffer const &, OutputParams const &);


} // namespace lyx

#endif // INSET_BIBITEM_H
