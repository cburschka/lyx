// -*- C++ -*-

#ifndef PARAGRAPHPARAMETERS_H
#define PARAGRAPHPARAMETERS_H

#include "ShareContainer.h"
#include "LString.h"
#include "layout.h"

class VSpace;
class Spacing;
struct ParameterStruct;

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
	void makeSame(ParagraphParameters const & pp);
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
	char depth() const;
	///
	void depth(char);
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
	int pextraType() const;
	///
	void pextraType(int);
	///
	string const & pextraWidth() const;
	///
	void pextraWidth(string const &);
	///
	string const & pextraWidthp() const;
	///
	void pextraWidthp(string const &);
	///
	int pextraAlignment() const;
	///
	void pextraAlignment(int);
	///
	bool pextraHfill() const;
	///
	void pextraHfill(bool);
	///
	bool pextraStartMinipage() const;
	///
	void pextraStartMinipage(bool);
private:
	///
	void set_from_struct(ParameterStruct const &);
	///
	boost::shared_ptr<ParameterStruct> param;
	///
	static ShareContainer<ParameterStruct> container;
};
#endif
