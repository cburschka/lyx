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
class XMLStream;

/// This class contains the macro definition.
class InsetMathMacroTemplate : public InsetMathNest {
public:
	///
	explicit InsetMathMacroTemplate(Buffer * buf);
	///
	InsetMathMacroTemplate(Buffer * buf, docstring const & name, int numargs,
		int optionals, MacroType type,
		std::vector<MathData> const & optionalValues = std::vector<MathData>(),
		MathData const & def = MathData(),
		MathData const & display = MathData());
	/// parses from string, returns false if failed
	bool fromString (const docstring & str);
	///
	bool editable() const override { return true; }
	///
	void edit(Cursor & cur, bool front, EntryDirection entry_from) override;
	///
	bool notifyCursorLeaves(Cursor const & old, Cursor & cur) override;
	///
	void read(Lexer & lex) override;
	///
	void write(std::ostream & os) const override;
	///
	void write(WriteStream & os) const override;
	/// Output LaTeX code, but assume that the macro is not defined yet
	/// if overwriteRedefinition is true
	int write(WriteStream & os, bool overwriteRedefinition) const;
	/// Nothing happens. This is simply to suppress the default output.
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	int plaintext(odocstringstream &, OutputParams const &, size_t) const override;
	///
	bool inheritFont() const override { return false; }
	///
	docstring name() const override;
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
	void validate(LaTeXFeatures &) const override;

	/// decide whether its a redefinition
	void updateToContext(MacroContext const & mc);

	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	/// identifies macro templates
	InsetMathMacroTemplate * asMacroTemplate() override { return this; }
	/// identifies macro templates
	InsetMathMacroTemplate const * asMacroTemplate() const override { return this; }
	///
	InsetCode lyxCode() const override { return MATHMACRO_CODE; }
	///
	void infoize(odocstream & os) const override;
	///
	std::string contextMenuName() const override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
protected:
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	/// do we want to handle this event?
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const override;

private:
	friend class InsetLabelBox;
	friend class InsetDisplayLabelBox;

	///
	Inset * clone() const override;

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
