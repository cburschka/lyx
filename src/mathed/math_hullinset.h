// -*- C++ -*-
/**
 * \file math_hullinset.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author André Pönitz
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef MATH_HULLINSET_H
#define MATH_HULLINSET_H

#include "math_gridinset.h"
#include <boost/scoped_ptr.hpp>

class RenderPreview;


/// This provides an interface between "LyX insets" and "LyX math insets"
class MathHullInset : public MathGridInset {
public:
	///
	MathHullInset();
	///
	explicit MathHullInset(std::string const & type);
	///
	~MathHullInset();
	///
	MathHullInset & operator=(MathHullInset const &);
	///
	mode_type currentMode() const;
	///
	void metrics(MetricsInfo & mi, Dimension & dim) const;
	///
	void draw(PainterInfo &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi, Dimension & dim) const;
	///
	void drawT(TextPainter &, int x, int y) const;
	///
	std::string label(row_type row) const;
	///
	void label(row_type row, std::string const & label);
	///
	void numbered(row_type row, bool num);
	///
	bool numbered(row_type row) const;
	///
	bool numberedType() const;
	///
	bool ams() const;
	/// Appends \c list with all labels found within this inset.
	void getLabelList(Buffer const &,
			  std::vector<std::string> & list) const;
	///
	void validate(LaTeXFeatures & features) const;
	/// identifies MatrixInsets
	MathHullInset const * asHullInset() const { return this; }
	/// identifies HullInset
	MathHullInset * asHullInset() { return this; }

	/// add a row
	void addRow(row_type row);
	/// delete a row
	void delRow(row_type row);
	/// swap two rows
	void swapRow(row_type row);
	/// add a column
	void addCol(col_type col);
	/// delete a column
	void delCol(col_type col);

	/// get type
	std::string const & getType() const;
	/// change type
	void mutate(std::string const &);

	///
	int defaultColSpace(col_type col);
	///
	char defaultColAlign(col_type col);
	///
	bool idxFirst(LCursor &) const;
	///
	bool idxLast(LCursor &) const;

	///
	void write(WriteStream & os) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void normalize(NormalStream &) const;
	///
	void infoize(std::ostream & os) const;

	///
	void write(Buffer const &, std::ostream & os) const;
	///
	void read(Buffer const &, LyXLex & lex);
	///
	int plaintext(Buffer const &, std::ostream &,
		  OutputParams const &) const;
	///
	int linuxdoc(Buffer const &, std::ostream &,
		     OutputParams const &) const;
	///
	int docbook(Buffer const &, std::ostream &,
		    OutputParams const &) const;

	/// get notification when the cursor leaves this inset
	void notifyCursorLeaves(LCursor & cur);
	///
	//bool insetAllowed(Code code) const;
	///
	void addPreview(lyx::graphics::PreviewLoader &) const;


protected:
	MathHullInset(MathHullInset const &);

	virtual void priv_dispatch(LCursor & cur, FuncRequest & cmd);

	/// do we want to handle this event?
	bool getStatus(LCursor & cur, FuncRequest const & cmd,
		FuncStatus & status) const;
	///
	std::string eolString(row_type row, bool fragile) const;

private:
	virtual std::auto_ptr<InsetBase> doClone() const;
	///
	void setType(std::string const & type);
	///
	void validate1(LaTeXFeatures & features);
	///
	void header_write(WriteStream &) const;
	///
	void footer_write(WriteStream &) const;
	///
	std::string nicelabel(row_type row) const;
	///
	void doExtern(LCursor & cur, FuncRequest & func);
	///
	void glueall();
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
	char const * standardFont() const;
	/// consistency check
	void check() const;
	/// can this change its number of rows?
	bool rowChangeOK() const;
	/// can this change its number of cols?
	bool colChangeOK() const;

	/// "none", "simple", "display", "eqnarray",...
	std::string type_;
	///
	std::vector<int> nonum_;
	///
	std::vector<std::string> label_;
	///
	boost::scoped_ptr<RenderPreview> preview_;
//
// Incorporate me
//
public:
	/// what appears in the minibuffer when opening
	virtual std::string const editMessage() const;
	///
	virtual void getCursorDim(int &, int &) const;
	///
	virtual bool isTextInset() const { return true; }
	///
	virtual void mutateToText();
	///
	virtual void revealCodes(LCursor & cur) const;
	///
	EDITABLE editable() const { return HIGHLY_EDITABLE; }
	///
	void edit(LCursor & cur, bool left);
	///
	bool display() const;
	///
	Code lyxCode() const;

protected:
	///
	void handleFont(LCursor &, std::string const & arg, std::string const & font);
	///
	void handleFont2(LCursor &, std::string const & arg);
};

#endif
