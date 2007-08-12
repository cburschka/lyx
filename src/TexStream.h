#ifndef LATEXSTREAM_H
#define LATEXSTREAM_H

#include "support/docstring.h"

#include "TexRow.h"

#include <iostream>
#include <streambuf>

namespace lyx {

class TexStreamBuffer;
class TexRow;

typedef std::basic_streambuf<char_type> TexStreamBase;

class TexStream : public std::basic_ostream<char_type>
{
public:
  TexStream(TexStreamBase * sbuf, TexRow * texrow);
  ~TexStream();
	int line() const;

private:
	TexStreamBuffer * sbuf_;
};

} // namespace lyx

#endif
