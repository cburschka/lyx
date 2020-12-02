// -*- C++ -*-
/**
 * \file InsetScript.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Georg Baum
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef INSET_SCRIPT_H
#define INSET_SCRIPT_H

#include "FontInfo.h"
#include "InsetText.h"


namespace lyx {

class InsetScriptParams
{
public:
	enum Type {
		Subscript,
		Superscript
	};
	/// \c type defaults to Subscript
	InsetScriptParams();
	///
	void write(std::ostream & os) const;
	///
	void read(Lexer & lex);
	///
	int shift(FontInfo const & font) const;
	///
	Type type;
};


/////////////////////////////////////////////////////////////////////////
//
// InsetScript
//
/////////////////////////////////////////////////////////////////////////

/// The subscript and superscript inset
class InsetScript : public InsetText
{
public:
	///
	InsetScript(Buffer *, InsetScriptParams const & = InsetScriptParams());
	///
	InsetScript(Buffer *, std::string const &);
	///
	~InsetScript();
	///
	static std::string params2string(InsetScriptParams const &);
	///
	static void string2params(std::string const &, InsetScriptParams &);
	///
	InsetScriptParams const & params() const { return params_; }

	/// \name Public functions inherited from Inset class
	//@{
	///
	InsetCode lyxCode() const override { return SCRIPT_CODE; }
	///
	docstring layoutName() const override;

	///
	int topOffset(BufferView const *) const override { return 0; }
	///
	int bottomOffset(BufferView const *) const override { return 0; }
	///
	int leftOffset(BufferView const *) const override { return 0; }
	///
	int rightOffset(BufferView const *) const override { return 0; }

	///
	void metrics(MetricsInfo &, Dimension &) const override;
	///
	void draw(PainterInfo & pi, int x, int y) const override;
	///
	void cursorPos(BufferView const & bv,
		CursorSlice const & sl, bool boundary, int & x, int & y) const override;
	///
	void write(std::ostream &) const override;
	///
	void read(Lexer & lex) override;
	///
	bool forcePlainLayout(idx_type = 0) const override { return true; }
	///
	bool allowParagraphCustomization(idx_type = 0) const override { return false; }
	///
	bool neverIndent() const override { return true; }
	///
	bool inheritFont() const override { return true; }
	///
	int plaintext(odocstringstream & ods, OutputParams const & op,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	void edit(Cursor & cur, bool front,
	          EntryDirection entry_from = ENTRY_DIRECTION_IGNORE) override;
	///
	Inset * editXY(Cursor & cur, int x, int y) override;
	///
	bool insetAllowed(InsetCode code) const override;
	///
	bool getStatus(Cursor &, FuncRequest const &, FuncStatus &) const override;
	///
	docstring toolTip(BufferView const & bv, int x, int y) const override;
	///
	Inset * clone() const override { return new InsetScript(*this); }
	///
	std::string contextMenuName() const override;
	//@}

	/// \name Public functions inherited from InsetText class
	//@{
	///
	bool allowMultiPar() const override { return false; }
	//@}

protected:
	/// \name Protected functions inherited from Inset class
	//@{
	///
	void doDispatch(Cursor & cur, FuncRequest & cmd) override;
	//@}

private:
	/// used by the constructors
	void init();
	///
	friend class InsetScriptParams;
	///
	InsetScriptParams params_;
	/// The font of containing inset; this is necessary to compute shift
	mutable FontInfo outer_font_;
};


} // namespace lyx

#endif
