// -*- C++ -*-
/**
 * \file math_macrotemplate.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Alejandro Aguilar Sierra
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_MACROTEMPLATE_H
#define MATH_MACROTEMPLATE_H

#include "InsetMathNest.h"
#include "MacroTable.h"
#include "MathData.h"


namespace lyx {

class OutputParams;
class XHTMLStream;

/// This class contains the macro definition.
class MathMacroTemplate : public InsetMathNest {
public:
	///
	MathMacroTemplate(Buffer * buf);
	///
	MathMacroTemplate(Buffer * buf, docstring const & name, int nargs,
		int optional, MacroType type, 
		std::vector<MathData> const & optionalValues = std::vector<MathData>(),
		MathData const & def = MathData(),
		MathData const & display = MathData());
	/// parses from string, returns false if failed
	bool fromString (const docstring & str);
	///
	bool editable() const { return true; }
	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from);
	///
	bool notifyCursorLeaves(Cursor const & old, Cursor & cur);
	///
	void read(Lexer & lex);
	///
	void write(std::ostream & os) const;
	///
	void write(WriteStream & os) const;
	/// Output LaTeX code, but assume that the macro is not definied yet
	/// if overwriteRedefinition is true
	int write(WriteStream & os, bool overwriteRedefinition) const;
	/// Nothing happens. This is simply to suppress the default output.
	docstring xhtml(XHTMLStream &, OutputParams const &) const;
	///
	int plaintext(odocstringstream &, OutputParams const &, size_t) const;
	///
	bool inheritFont() const { return false; }

	///
	docstring name() const;
	///
	void getDefaults(std::vector<docstring> & defaults) const;
	///
	docstring definition() const;
	///
	docstring displayDefinition() const;
	///
	size_t numArgs() const;
	///
	size_t numOptionals() const;
	///
	bool redefinition() const { return redefinition_; }
	///
	MacroType type() const { return type_; }

	/// check name and possible other formal properties
	bool validMacro() const;
	///
	bool validName() const;
	/// Remove everything from the name which makes it invalid 
	/// and return true iff it is valid.
	bool fixNameAndCheckIfValid();
	
	/// request "external features"
	virtual void validate(LaTeXFeatures &) const;

	/// decide whether its a redefinition
	void updateToContext(MacroContext const & mc);

	///
	void draw(PainterInfo & pi, int x, int y) const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	/// identifies macro templates
	MathMacroTemplate * asMacroTemplate() { return this; }
	/// identifies macro templates
	MathMacroTemplate const * asMacroTemplate() const { return this; }
	///
	InsetCode lyxCode() const { return MATHMACRO_CODE; }
	///
	void infoize(odocstream & os) const;
	///
	std::string contextMenuName() const;
protected:
	///
	virtual void doDispatch(Cursor & cur, FuncRequest & cmd);
	/// do we want to handle this event?
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const;

private:
	friend class InsetLabelBox;
	friend class DisplayLabelBox;
	
	///
	virtual Inset * clone() const;

	/// remove #n with from<=n<=to
	void removeArguments(Cursor & cur, DocIterator const & inset_pos,
		int from, int to);
	/// shift every #n with from<=n, i.e. #n -> #(n-by)
	void shiftArguments(size_t from, int by);
	///
	void insertParameter(Cursor & cur, DocIterator const & inset_pos,
		int pos, bool greedy = false, bool addarg = true); 
	///
	void removeParameter(Cursor & cur, DocIterator const & inset_pos,
		int pos, bool greedy = false);
	///
	void makeOptional(Cursor & cur, DocIterator const & inset_pos);
	///
	void makeNonOptional(Cursor & cur, DocIterator const & inset_pos);
	///
	idx_type defIdx() const { return optionals_ + 1; }
	/// index of default value cell of optional parameter (#1 -> n=0)
	idx_type optIdx(idx_type n) const { return n + 1; }
	///
	idx_type displayIdx() const { return optionals_ + 2; }
	///
	void updateLook() const;
	/// look through the macro for #n arguments
	int maxArgumentInDefinition() const;
	/// add missing #n arguments up to \c maxArg
	void insertMissingArguments(int maxArg);
	/// change the arity
	void changeArity(Cursor & cur, DocIterator const & inset_pos,
		int newNumArg);
	/// find arguments in definition and adapt the arity accordingly
	void commitEditChanges(Cursor & cur, DocIterator const & inset_pos);
	/// The representation of the macro template, with some holes to edit
	mutable MathData look_;
	///
	mutable int numargs_;
	///
	mutable int argsInLook_;
	///
	int optionals_;
	/// keeps the old optional default value when an 
	/// optional argument is disabled
	std::vector<MathData> optionalValues_;

	/// (re)newcommand or def
	mutable MacroType type_;
	/// defined before already?
	bool redefinition_;
	///
	void createLook(int args) const;
	///
	mutable bool lookOutdated_;
	/// true if in pre-calculations of metrics to get height of boxes
	mutable bool premetrics_;
	///
	mutable int labelBoxAscent_;
	///
	mutable int labelBoxDescent_;
	///
	bool premetrics() const { return premetrics_; }
	///
	int commonLabelBoxAscent() const { return labelBoxAscent_; }
	///
	int commonLabelBoxDescent() const { return labelBoxDescent_; }
};


} // namespace lyx

#endif
