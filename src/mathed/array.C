
#ifdef __GNUG__
#pragma implementation
#endif

#include "math_inset.h"
#include "debug.h"
#include "array.h"
#include "math_scriptinset.h"
#include "math_parser.h"
#include "mathed/support.h"

using std::ostream;
using std::endl;

MathArray::MathArray()
{}


MathArray::~MathArray()
{
	erase();
}


MathArray::MathArray(MathArray const & array)
	: bf_(array.bf_)
{
	deep_copy(0, size());
}


MathArray::MathArray(MathArray const & array, int from, int to)
	: bf_(array.bf_.begin() + from, array.bf_.begin() + to)
{
	deep_copy(0, size());
}


void MathArray::deep_copy(int pos1, int pos2)
{
	for (int pos = pos1; pos < pos2; next(pos)) 
		if (isInset(pos)) {
			MathInset * p = nextInset(pos)->clone();
			memcpy(&bf_[pos + 1], &p, sizeof(p));
		}
}


bool MathArray::next(int & pos) const
{
	if (pos >= size() - 1)
		return false;

	pos += item_size(pos);
	return true;
}


bool MathArray::prev(int & pos) const
{
	if (pos == 0)
		return false;

	pos -= item_size(pos - 1);
	return true;
}


bool MathArray::last(int & pos) const
{
	pos = bf_.size();
	return prev(pos);
}


int MathArray::item_size(int pos) const
{
	return 2 + (isInset(pos) ? sizeof(MathInset*) : 1);
}


void MathArray::substitute(MathMacro const & m)
{
	MathArray tmp;
	for (int pos = 0; pos < size(); next(pos)) {
		if (isInset(pos)) 
			nextInset(pos)->substitute(tmp, m);
		else 
			tmp.push_back(getChar(pos), getCode(pos));
	}
	swap(tmp);
}


MathArray & MathArray::operator=(MathArray const & array)
{
	MathArray tmp(array);
	swap(tmp);
	return *this;
}


MathInset * MathArray::nextInset(int pos) const
{
	if (!isInset(pos))
		return 0;
	MathInset * p;
	memcpy(&p, &bf_[0] + pos + 1, sizeof(p));
	return p;
}


MathInset * MathArray::prevInset(int pos) const
{
	if (!prev(pos))
		return 0;
	return nextInset(pos);
}


unsigned char MathArray::getChar(int pos) const
{
	return pos < size() ? bf_[pos + 1] : '\0';
}


string MathArray::getString(int & pos) const
{
	string s;
	if (isInset(pos))
		return s;

	MathTextCodes const fcode = getCode(pos);
	do {
		s += getChar(pos);
		next(pos);
	} while (pos < size() && !isInset(pos) && getCode(pos) == fcode);

	return s;
}


MathTextCodes MathArray::getCode(int pos) const
{
	return pos < size() ? MathTextCodes(bf_[pos]) : LM_TC_MIN;
}


void MathArray::setCode(int pos, MathTextCodes t)
{
	if (pos > size() || isInset(pos))
		return;
	bf_[pos] = t;
	bf_[pos + 2] = t;
}



void MathArray::insert(int pos, MathInset * p)
{
	bf_.insert(bf_.begin() + pos, 2 + sizeof(p), LM_TC_INSET);
	memcpy(&bf_[pos + 1], &p, sizeof(p));
}


void MathArray::insert(int pos, unsigned char b, MathTextCodes t)
{
	bf_.insert(bf_.begin() + pos, 3, t);
	bf_[pos + 1] = b;
}


void MathArray::insert(int pos, MathArray const & array)
{
	bf_.insert(bf_.begin() + pos, array.bf_.begin(), array.bf_.end());
	deep_copy(pos, pos + array.size());
}


void MathArray::push_back(MathInset * p)
{	
	insert(size(), p);
}


void MathArray::push_back(unsigned char b, MathTextCodes c)
{
	insert(size(), b, c);
}


void MathArray::push_back(MathArray const & array)
{
	insert(size(), array);
}


void MathArray::clear()
{
	erase();
}


void MathArray::swap(MathArray & array)
{
	if (this != &array) 
		bf_.swap(array.bf_);
}


bool MathArray::empty() const
{
	return bf_.empty();
}
   

int MathArray::size() const
{
	return bf_.size();
}


void MathArray::erase()
{
	erase(0, size());
}


void MathArray::erase(int pos)
{
	if (pos < static_cast<int>(bf_.size()))
		erase(pos, pos + item_size(pos));
}


void MathArray::erase(int pos1, int pos2)
{
	for (int pos = pos1; pos < pos2; next(pos))
		if (isInset(pos))
			delete nextInset(pos);
	bf_.erase(bf_.begin() + pos1, bf_.begin() + pos2);
}


bool MathArray::isInset(int pos) const
{
	if (pos >= size())
		return false;
	return MathIsInset(static_cast<MathTextCodes>(bf_[pos]));
}


MathInset * MathArray::back_inset() const
{
	return prevInset(size());
}


void MathArray::dump2(ostream & os) const
{
	for (buffer_type::const_iterator it = bf_.begin(); it != bf_.end(); ++it)
		os << int(*it) << ' ';
	os << endl;
}


void MathArray::dump(ostream & os) const
{
	for (int pos = 0; pos < size(); next(pos)) {
		if (isInset(pos)) 
			os << "<inset: " << nextInset(pos) << ">";
		else 
			os << "<" << int(bf_[pos]) << " " << int(bf_[pos+1]) << ">";
	}
}


std::ostream & operator<<(std::ostream & os, MathArray const & ar)
{
	ar.dump2(os);
	return os;
}


void MathArray::write(ostream & os, bool fragile) const
{
	if (empty())
		return;

	int brace = 0;
	
	for (int pos = 0; pos < size(); next(pos)) {
		if (isInset(pos)) {

			nextInset(pos)->write(os, fragile);

		} else {

			MathTextCodes fcode = getCode(pos);
			unsigned char c = getChar(pos);

			if (MathIsSymbol(fcode)) {
				latexkeys const * l = lm_get_key_by_id(c, LM_TK_SYM);

				if (l == 0) {
					l = lm_get_key_by_id(c, LM_TK_BIGSYM);
				}

				if (l) {
					os << '\\' << l->name << ' ';
				} else {
					lyxerr << "Could not find the LaTeX name for  " << c << " and fcode " << fcode << "!" << std::endl;
				}
			} else {
				if (fcode >= LM_TC_RM && fcode <= LM_TC_TEXTRM) 
					os << '\\' << math_font_name[fcode - LM_TC_RM] << '{';

				// Is there a standard logical XOR?
				if ((fcode == LM_TC_TEX && c != '{' && c != '}') ||
						(fcode == LM_TC_SPECIAL))
					os << '\\';
				else {
					if (c == '{')
						++brace;
					if (c == '}')
						--brace;
				}
				if (c == '}' && fcode == LM_TC_TEX && brace < 0) 
					lyxerr <<"Math warning: Unexpected closing brace.\n";
				else	       
					os << c;
			}

			if (fcode >= LM_TC_RM && fcode <= LM_TC_TEXTRM)
				os << '}';
			
		}
	}

	if (brace > 0)
		os << string(brace, '}');
}


void MathArray::writeNormal(ostream & os) const
{
	if (empty()) {
		os << "[par] ";
		return;
	}

	write(os, true);
}


void MathArray::validate(LaTeXFeatures & features) const
{
	for (int pos = 0; pos < size(); next(pos)) 
		if (isInset(pos)) 
			nextInset(pos)->validate(features);
}


void MathArray::pop_back()
{	
	int pos = size();
	prev(pos);
	erase(pos, size());
}

