// -*- C++ -*-
#ifndef MATH_HULLINSET_H
#define MATH_HULLINSET_H

#include "math_gridinset.h"

#ifdef __GNUG__
#pragma interface
#endif

/** This provides an interface between "LyX insets" and "LyX math insets"
 *  \author André Pönitz
 */

class LaTeXFeatures;

class MathHullInset : public MathGridInset {
public:
	///
	MathHullInset();
	///
	explicit MathHullInset(string const & type);
	///
	MathInset * clone() const;
	///
	mode_type currentMode() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & mi) const;
	///
	void drawT(TextPainter &, int x, int y) const;
	///
	string label(row_type row) const;
	///
	void label(row_type row, string const & label);
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
	/// local dispatcher
	result_type dispatch(FuncRequest const & cmd, idx_type & idx, pos_type & pos);
	/// 
	void getLabelList(std::vector<string> &) const;
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
	/// add a column
	void addCol(col_type col);
	/// delete a column
	void delCol(col_type col);

	/// get type
	string const & getType() const;
	/// change type
	void mutate(string const &);

	///
	int defaultColSpace(col_type col);
	///
	char defaultColAlign(col_type col);
	///
	bool idxFirst(idx_type &, pos_type &) const;
	///
	bool idxLast(idx_type &, pos_type &) const;

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
	string eolString(row_type row, bool fragile) const;

private:
	///
	void setType(string const & type);
	///
	void validate1(LaTeXFeatures & features);
	///
	void header_write(WriteStream &) const;
	///
	void footer_write(WriteStream &) const;
	///
	string nicelabel(row_type row) const;
	///
	void doExtern(FuncRequest const & func, idx_type & idx, pos_type & pos);
	///
	void glueall();
	///
	char const * standardFont() const;
	/// consistency check
	void check() const;
	/// can this change its number of cols?
	bool colChangeOK() const;

	/// "none", "simple", "display", "eqnarray",...
	string type_;
	///
	std::vector<int> nonum_;
	///
	std::vector<string> label_;
};

#endif
