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


class LaTeXFeatures;

/// This provides an interface between "LyX insets" and "LyX math insets"
class MathHullInset : public MathGridInset {
public:
	///
	MathHullInset();
	///
	explicit MathHullInset(std::string const & type);
	///
	virtual std::auto_ptr<InsetBase> clone() const;
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
	bool display() const;
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
	///
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
	std::string fileInsetLabel() const;
	///
	void write(WriteStream & os) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void normalize(NormalStream &) const;
	///
	void infoize(std::ostream & os) const;

protected:
	///
	virtual
	DispatchResult
	priv_dispatch(BufferView & bv, FuncRequest const & cmd);
	///
	std::string eolString(row_type row, bool fragile) const;

private:
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
	void doExtern(LCursor & cur, FuncRequest const & func);
	///
	void glueall();
	///
	char const * standardFont() const;
	/// consistency check
	void check() const;
	/// can this change its number of cols?
	bool colChangeOK() const;

	/// "none", "simple", "display", "eqnarray",...
	std::string type_;
	///
	std::vector<int> nonum_;
	///
	std::vector<std::string> label_;
};

#endif
