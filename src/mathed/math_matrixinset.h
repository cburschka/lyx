// -*- C++ -*-
#ifndef MATH_MATRIXINSET_H
#define MATH_MATRIXINSET_H

#include <vector>

#include "math_parinset.h"

/** Multiline math paragraph base class.
    This is the base to all multiline editable math objects
    like array and eqnarray. 
 */
class MathMatrixInset: public MathParInset {
 public: 
    ///
    explicit
    MathMatrixInset(int m = 1, int n = 1, short st = LM_ST_TEXT);
    ///
    explicit
    MathMatrixInset(MathMatrixInset *);
    ///
    MathedInset * Clone();
    ///
    virtual ~MathMatrixInset();
    ///
    void draw(Painter &, int, int);
    ///
    void Write(std::ostream &, bool fragile);
    ///
    void Metrics();
    ///
    void SetData(MathedArray *);
    ///
    void SetAlign(char, string const &);
    ///
    int GetColumns() const { return nc; }
    ///
    int GetRows() const { return nr; }
    ///
    virtual bool isMatrix() const { return true; }

    /// Use this to manage the extra information independently of paragraph
    MathedRowSt * getRowSt() const { return row; }
    ///
    void setRowSt(MathedRowSt * r) { row = r; }
    
 protected:
    ///  Number of columns & rows
    int nc;
    ///
    int nr;
    /// tab sizes
    std::vector<int> ws_;   
    /// 
    char v_align; // add approp. type
    ///
	//std::vector<char> h_align;
	string h_align; // a vector would perhaps be more correct
    /// Vertical structure
    MathedRowSt * row;

};
#endif
