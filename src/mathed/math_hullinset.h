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
	explicit MathHullInset(MathInsetTypes t);
	///
	MathHullInset(MathInsetTypes t, MathGridInset const & grid);
	///
	MathHullInset(MathInsetTypes t, col_type cols);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & st) const;
	///
	void draw(MathPainterInfo &, int x, int y) const;
	///
	void metricsT(TextMetricsInfo const & st) const;
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
	///
	std::vector<string> const getLabelList() const;
	///
	void validate(LaTeXFeatures & features) const;
	/// identifies MatrixInsets
	MathHullInset const * asHullInset() const { return this; }
	/// identifies HullInset
	MathHullInset * asHullInset() { return this; }

	///
	void addRow(row_type);
	///
	void delRow(row_type);
	///
	void addFancyCol(col_type);
	///
	void delFancyCol(col_type);

	/// change type
	void mutate(string const &);
	///
	void mutate(MathInsetTypes);

	///
	int defaultColSpace(col_type col);
	///
	char defaultColAlign(col_type col);
	///
	bool idxFirst(idx_type &, pos_type &) const;
	///
	bool idxLast(idx_type &, pos_type &) const;

	///
	MathInsetTypes getType() const;
	///
	void write(WriteStream & os) const;
	///
	void mathmlize(MathMLStream &) const;
	///
	void normalize(NormalStream &) const;
	///
	void infoize(std::ostream & os) const;

private:
	///
	void setType(MathInsetTypes t);
	///
	void validate1(LaTeXFeatures & features);
	///
	void header_write(WriteStream &) const;
	///
	void footer_write(WriteStream &) const;
	///
	void glueall();
	///
	string nicelabel(row_type row) const;
	/// consistency check
	void check() const;

	///
	MathInsetTypes objtype_;
	///
	std::vector<int> nonum_;
	///
	std::vector<string> label_;
};

#endif
