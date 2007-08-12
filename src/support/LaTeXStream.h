#ifndef LATEXSTREAM_H
#define LATEXSTREAM_H

#include <iostream>
#include <streambuf>

namespace lyx {

class LaTeXStreamBuffer;

class LaTeXStream : public std::ostream
{
public:
  LaTeXStream(std::streambuf * sbuf);
  ~LaTeXStream();
	int line() const;
private:
	LaTeXStreamBuffer * sbuf_;
};

} // namespace lyx

#endif
