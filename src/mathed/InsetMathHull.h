// -*- C++ -*-
/**
 * \file InsetMathHull.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_HULLINSET_H
#define MATH_HULLINSET_H

#include "InsetMathGrid.h"

#include "DocIterator.h"
#include "OutputEnums.h"

#include "support/unique_ptr.h"


namespace lyx {

class InsetLabel;
class MacroNameSet;
class ParConstIterator;
class RenderPreview;


/// This provides an interface between "LyX insets" and "LyX math insets"
class InsetMathHull : public InsetMathGrid {
public:
	/// How a line is numbered
	enum Numbered {
		/// not numbered, LaTeX code \\nonumber if line differs from inset
		NONUMBER,
		/// numbered, LaTeX code \\number if line differs from inset
		NUMBER,
		/// not numbered, LaTeX code \\notag if line differs from inset
		NOTAG
	};
	///
	explicit InsetMathHull(Buffer * buf);
	///
	InsetMathHull(Buffer * buf, HullType type);
	///
	virtual ~InsetMathHull();
	///
	void setBuffer(Buffer &) override;
	///
	void updateBuffer(ParIterator const &, UpdateType, bool const deleted = false) override;
	///
	void addToToc(DocIterator const & di, bool output_active,
				  UpdateType utype, TocBackend & backend) const override;
	///
	InsetMathHull & operator=(InsetMathHull const &);
	///
	mode_type currentMode() const override;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const override;
	///
	void drawBackground(PainterInfo & pi, int x, int y) const override;
	///
	void draw(PainterInfo &, int x, int y) const override;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const override;
	///
	void drawT(TextPainter &, int x, int y) const override;
	///
	docstring label(row_type row) const;
	///
	void label(row_type row, docstring const & label);
	///
	std::vector<InsetLabel *> const & getLabels() { return label_; }
	///
	ColorCode backgroundColor(PainterInfo const &) const override;
	///
	void numbered(row_type row, bool num) { numbered(row, num ? NUMBER : NONUMBER); }
	///
	void numbered(row_type row, Numbered num);
	///
	bool numbered(row_type row) const;
	///
	bool numberedType() const override;
	///
	bool ams() const;
	///
	bool outerDisplay() const;
	///
	CtObject getCtObject(OutputParams const &) const override;
	///
	void validate(LaTeXFeatures & features) const override;
	/// identifies HullInset
	InsetMathHull const * asHullInset() const override { return this; }
	/// identifies HullInset
	InsetMathHull * asHullInset() override { return this; }

	/// add a row
	void addRow(row_type row) override;
	/// delete a row
	void delRow(row_type row) override;
	/// swap two rows
	void swapRow(row_type row) override;
	/// add a column
	void addCol(col_type col) override;
	/// delete a column
	void delCol(col_type col) override;

	/// get type
	HullType getType() const override;
	/// is mutation implemented for this type?
	static bool isMutable(HullType type);
	/// change type
	void mutate(HullType newtype) override;

	///
	int defaultColSpace(col_type col) override;
	///
	int displayColSpace(col_type col) const override;
	///
	char defaultColAlign(col_type col) override;
	///
	char displayColAlign(idx_type idx) const override;

	///
	void write(TeXMathStream & os) const override;
	///
	void normalize(NormalStream &) const override;
	///
	void infoize(odocstream & os) const override;

	///
	void write(std::ostream & os) const override;
	///
	void header_write(TeXMathStream &) const;
	///
	void footer_write(TeXMathStream &) const;
	///
	void read(Lexer & lex) override;
	///
	bool readQuiet(Lexer & lex);
	///
	int plaintext(odocstringstream &, OutputParams const &,
	              size_t max_length = INT_MAX) const override;
	///
	void docbook(XMLStream &, OutputParams const &) const override;
	///
	docstring xhtml(XMLStream &, OutputParams const &) const override;
	///
	void mathmlize(MathMLStream &) const override;
	///
	void htmlize(HtmlStream &) const override;
	///
	void mathAsLatex(TeXMathStream &) const;
	///
	void toString(odocstream &) const override;
	///
	void forOutliner(docstring &, size_t const, bool const) const override;

	/// get notification when the cursor leaves this inset
	bool notifyCursorLeaves(Cursor const & old, Cursor & cur) override;
	///
	//bool insetAllowed(InsetCode code) const override;
	///
	void addPreview(DocIterator const & inset_pos,
		graphics::PreviewLoader &) const override;
	/// Recreates the preview if preview is enabled.
	void reloadPreview(DocIterator const & pos) const;
	///
	void usedMacros(MathData const & md, DocIterator const & pos,
	                MacroNameSet & macros, MacroNameSet & defs) const;
	///
	void initUnicodeMath() const override;

	/// Force inset into LTR environment if surroundings are RTL
	bool forceLTR(OutputParams const &) const override { return true; }
	///
	void recordLocation(DocIterator const & di);

	///
	std::string contextMenuName() const override;
	///
	InsetCode lyxCode() const override { return MATH_HULL_CODE; }
	///
	bool canPaintChange(BufferView const &) const override;
	///
	bool confirmDeletion() const override { return nargs() != 1 || !cell(0).empty(); }

protected:
	InsetMathHull(InsetMathHull const &);

	void doDispatch(Cursor & cur, FuncRequest & cmd) override;

	/// do we want to handle this event?
	bool getStatus(Cursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const override;
	/// override to set to 0 for inline equation
	int leftMargin() const override;
	/// override to set to 0 for inline equation
	int rightMargin() const override;
	/// override to set to 0 for inline equation
	int border() const override;
	///
	docstring eolString(row_type row, bool fragile, bool latex,
			bool last_eoln) const override;

private:
	Inset * clone() const override;
	/// Prepare the preview if preview is enabled.
	/// \param forexport: whether this is intended for export
	/// If so, we ignore LyXRC and wait for the image to be generated.
	void preparePreview(DocIterator const & pos,
	                    bool forexport = false) const;
	/// like reloadPreview, but forces load
	/// used by image export
	void loadPreview(DocIterator const & pos) const;
	///
	void setType(HullType type);
	///
	void validate1(LaTeXFeatures & features);
	///
	docstring nicelabel(row_type row) const;
	///
	void doExtern(Cursor & cur, FuncRequest & func);
	///
	void glueall(HullType type);
	/*!
	 * split every row at the first relation operator.
	 * The number of columns must be 1. One column is added.
	 * The first relation operator and everything after it goes to the
	 * second column.
	 */
	void splitTo2Cols();
	/*!
	 * split every row at the first relation operator.
	 * The number of columns must be < 3. One or two columns are added.
	 * The first relation operator goes to the second column.
	 * Everything after it goes to the third column.
	 */
	void splitTo3Cols();
	/// change number of columns, split or combine columns if necessary.
	void changeCols(col_type);
	///
	std::string standardFont() const;
	///
	ColorCode standardColor() const;
	/// consistency check
	void check() const;
	/// does it understand tabular-feature commands?
	bool allowsTabularFeatures() const;
	/// can this change its number of rows?
	bool rowChangeOK() const;
	/// can this change its number of cols?
	bool colChangeOK() const;
	/// are any of the equations numbered?
	bool haveNumbers() const;
	/// draw four angular markers
	void drawMarkers(PainterInfo & pi, int x, int y) const override;

	/// "none", "simple", "display", "eqnarray",...
	HullType type_;
	///
	std::vector<Numbered> numbered_;
	///
	std::vector<docstring> numbers_;
	///
	std::vector<InsetLabel *> label_;
	///
	unique_ptr<RenderPreview> preview_;
	///
	DocIterator docit_;
	///
	typedef std::map<docstring, int> CounterMap;
	/// used to store current values of important counters
	CounterMap counter_map;
//
// Incorporate me
//
public:
	///
	virtual void mutateToText();
	///
	bool editable() const override { return true; }
	///
	void edit(Cursor & cur, bool front,
		EntryDirection entry_from = ENTRY_DIRECTION_IGNORE) override;
	///
	Inset * editXY(Cursor & cur, int x, int y) override;
	///
	int rowFlags() const override;
	/// helper function
	bool display() const { return rowFlags() & Display; }

	///
	int indent(BufferView const &) const override;

protected:
	///
	void handleFont(Cursor & cur, docstring const & arg,
		docstring const & font);
	///
	void handleFont2(Cursor & cur, docstring const & arg);
	///
	bool previewState(BufferView const * const bv) const;
};



} // namespace lyx
#endif
