// -*- C++ -*-
#ifndef MATH_BOXINSET_H
#define MATH_BOXINSET_H

#include "math_gridinset.h"
#include "LString.h"

#ifdef __GNUG__
#pragma interface
#endif

class LyXFont;

// Try to implement the reference inset "natively" for mathed.
// This is here temporarily until I can do cvs add again.

class ButtonInset: public MathNestInset {
public:
	///
	ButtonInset();
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;

protected:
	/// This should provide the text for the button
	virtual string screenLabel() const = 0;
};


// for things like \name[options]{contents}
class CommandInset : public ButtonInset {
public:
	/// name, contents, options deliminited by '|++|'
	explicit CommandInset(string const & data);
	///
	MathInset * clone() const;
	///
	void write(WriteStream & os) const;
	///
	//void infoize(std::ostream & os) const;
	///
	//int dispatch(string const & cmd, idx_type idx, pos_type pos);
	///
	string screenLabel() const;
public:
	string name_;
};


// for \ref 
class RefInset : public CommandInset {
public:
	///
	RefInset();
	///
	explicit RefInset(string const & data);
	///
	MathInset * clone() const;
	///
	//void write(WriteStream & os) const;
	///
	void infoize(std::ostream & os) const;
	///
	int dispatch(string const & cmd, idx_type idx, pos_type pos);
	///
	string screenLabel() const;
	///
	void validate(LaTeXFeatures & features) const;

	/// plain ascii output
	int ascii(std::ostream & os, int) const;
	/// linuxdoc output
	int linuxdoc(std::ostream & os) const;
	/// docbook output
	int docbook(std::ostream & os, bool) const;


	struct type_info {
		///
		string latex_name;
		///
		string gui_name;
		///
		string short_gui_name;
	};
	static type_info types[];
	///
	static int getType(string const & name);
	///
	static string const & getName(int type);
};


/// Support for \\mbox

class MathBoxInset : public MathGridInset {
public:
	///
	explicit MathBoxInset(string const & name);
	///
	MathInset * clone() const;
	///
	void metrics(MathMetricsInfo & mi) const;
	///
	void draw(MathPainterInfo & pi, int x, int y) const;
	/// identifies BoxInsets
	MathBoxInset * asBoxInset() { return this; }
	/// identifies BoxInsets
	MathBoxInset const * asBoxInset() const { return this; }
	///
	void rebreak();
	///
	void write(WriteStream & os) const;
	///
	void normalize(NormalStream & ns) const;

private:
	///
	mutable MathMetricsInfo mi_;
	///
	string name_;
};


#endif
