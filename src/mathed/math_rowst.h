// -*- C++ -*-
#ifndef MATH_ROWST_H
#define MATH_ROWST_H

#include "support/LAssert.h"
#include <vector>

/** The physical structure of a row and aditional information is stored here.
    It allows to manage the extra info independently of the paragraph data.  
    Only used for multiline paragraphs.
 */

class MathedRowStruct
{
public:
	///
	typedef std::vector<int> Widths;
	
	///
	MathedRowStruct();
	///
	string const & getLabel() const;
	///
	bool isNumbered() const;
	///
	int  getBaseline() const;
	///
	void setBaseline(int b);
	///
	int ascent() const;
	///
	int descent() const;
	///
	void ascent(int a);
	///
	void descent(int d);
	///
	int  getTab(unsigned int i) const;
	/// 
	void setLabel(string const & l);
	///
	void setNumbered(bool nf);
	///
	void setTab(unsigned int i, int t);
	///
	friend class MathedRowSt;
protected:
	/// Vericals 
	int asc_;
	///
	int desc_;
	///
	int y_;
	/// widths 
	Widths widths_;
	/// 
	string label_;
	///
	bool numbered_;
};


class MathedRowContainer {
public:
	///
	typedef std::vector<MathedRowStruct>   data_type;
	///
	typedef data_type::size_type           size_type;
	///
	struct iterator {
		///
		iterator();
		///
		explicit iterator(MathedRowContainer * m);
		/// "better" conversion to bool
		operator void *() const;
		///
		MathedRowStruct * operator->();
		///
		MathedRowStruct const * operator->() const;
		///
		void operator++();
		///
		bool is_last() const;
		///
		bool operator==(const iterator & it) const;

	//private:
		MathedRowContainer * st_;
		///
		unsigned int pos_;
	};

public:
	/// 
	iterator begin();
	/// 
	iterator end();
	///
	bool empty() const;

	/// insert item before 'it'
	void insert(iterator const & it);
	/// erase item pointed to by 'it'
	void erase(iterator & it);
	/// access to last row
	MathedRowStruct & back();
	/// append empty element
	void push_back();
	///
	size_type size() const;
	
//private:
	///
	std::vector<MathedRowStruct> data_;
};

#endif
