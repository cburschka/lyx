// -*- C++ -*-
#ifndef MATH_XITER_H
#define MATH_XITER_H

#include "math_iter.h"

class MathedRowSt;
class MathParInset;

/**
   A graphic iterator (updates position.) Used for
   metrics and updating cursor position
*/
class MathedXIter: public MathedIter {
public:
	///
	MathedXIter();
	//
	MathedXIter(MathParInset *);
	///
	void SetData(MathParInset *);
	///
	MathParInset * getPar() const;
	///
	bool Next();
	///
	bool Prev();
	///
	bool Up();
	///
	bool Down();
	///
	void goNextColumn();
	///
	void GoLast();
	///
	void GoBegin();
	///
	void Adjust();
	///
	void GetPos(int &, int &) const;
	///
	void GetIncPos(int &, int &) const;
	///
	string const GetString() const;
	///
	int GetX() const;
	///
	int GetY() const;
	///
	void subMetrics(int, int);
	///
	void fitCoord(int, int);
	/// 
	void getAD(int & a, int & d) const;
	
	/// Create a new row and insert #ncols# tabs.
	void addRow();
	///
	void delRow();
	
	///
	bool setLabel(string const & label);
	///
	static string error_label;
	///
	string const & getLabel() const;
	///
	bool setNumbered(bool);
	
	///
	void setTab(int, int);
	/// Merge the array at current position
	void Merge(MathedArray *);
	/// Delete every object from current position to pos2
	void Clean(int pos2);
	///
	MathedRowSt * adjustVerticalSt();
private:
	/// This function is not recursive, as MathPar::Metrics is
	void IMetrics(int, int &, int &, int &);
	/// Font size (display, text, script, script2) 
	int size;
	/// current position
	mutable int x;
	///
	int y;
	///
	MathParInset * p;
	
	// Limits auxiliary variables
	/// Position and max width of a script
	int sx;
	///
	int sw;
	/// true= center, false= left align (default)
	bool limits;
	///
	void ipush();
	///
	void ipop();
protected:
	/// 
	MathedRowSt * crow;
	
	///
	friend class MathedCursor;
};
#endif
