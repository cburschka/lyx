// -*- C++ -*-
/**
 * \file ParagraphParameters.h
 * This file is part of LyX, the document processor.
 * Licence details can be found in the file COPYING.
 *
 * \author Lars Gullik Bjønnes
 * \author Angus Leeming
 * \author John Levon
 *
 * Full author contact details are available in file CREDITS.
 */

#ifndef PARAGRAPHPARAMETERS_H
#define PARAGRAPHPARAMETERS_H

#include "support/std_string.h"
#include "ShareContainer.h"
#include "layout.h"

#include "ParameterStruct.h"

#include <iosfwd>

class VSpace;
class Spacing;
class LyXLex;
class Paragraph;


///
class ParagraphParameters {
public:
	///
	ParagraphParameters();
	///
	void clear();
	///
	bool sameLayout(ParagraphParameters const &) const;
	///
	VSpace const & spaceTop() const;
	///
	void spaceTop(VSpace const &);
	///
	VSpace const & spaceBottom() const;
	///
	void spaceBottom(VSpace const &);
	///
	Spacing const & spacing() const;
	///
	void spacing(Spacing const &);
	///
	bool noindent() const;
	///
	void noindent(bool);
	///
	bool lineTop() const;
	///
	void lineTop(bool);
	///
	bool lineBottom() const;
	///
	void lineBottom(bool);
	///
	bool pagebreakTop() const;
	///
	void pagebreakTop(bool);
	///
	bool pagebreakBottom() const;
	///
	void pagebreakBottom(bool);
	///
	LyXAlignment align() const;
	///
	void align(LyXAlignment);
	///
	typedef ParameterStruct::depth_type depth_type;
	///
	depth_type depth() const;
	///
	void depth(depth_type);
	///
	bool startOfAppendix() const;
	///
	void startOfAppendix(bool);
	///
	bool appendix() const;
	///
	void appendix(bool);
	///
	string const & labelString() const;
	///
	void labelString(string const &);
	///
	string const & labelWidthString() const;
	///
	void labelWidthString(string const &);
	///
	LyXLength const & leftIndent() const;
	///
	void leftIndent(LyXLength const &);

	/// read the parameters from a lex
	void read(LyXLex & lex);

	/// write out the parameters to a stream
	void write(std::ostream & os) const;

private:
	///
	void set_from_struct(ParameterStruct const &);
	///
	boost::shared_ptr<ParameterStruct> param;
	///
	static ShareContainer<ParameterStruct> container;
};


inline
ParagraphParameters::depth_type ParagraphParameters::depth() const
{
	return param->depth;
}


/** Generate a string \param data from \param par's ParagraphParameters.
    The function also generates some additional info needed by the
    Paragraph dialog.
 */
void params2string(Paragraph const & par, string & data);

/** Given \param data, an encoding of the ParagraphParameters generated
    in the Paragraph dialog, this function sets the current paragraph
    appropriately.
 */
void setParagraphParams(BufferView & bv, string const & data);

#endif
