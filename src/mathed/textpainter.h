#ifndef TEXTPAINTER_H
#define TEXTPAINTER_H

#include <vector>
#include <iosfwd>

class TextPainter {
	public:
		///
		TextPainter(int xmax, int ymax);
		///
		void draw(int x, int y, char const * str);
		///
		void draw(int x, int y, char c);
		///
		void show(std::ostream & os) const;
		///
		int textheight() const { return ymax_; }
		///
		void horizontalLine(int x, int y, int len, char c = '-');
		///
		void verticalLine(int x, int y, int len, char c = '|');

	private:
		///
		typedef std::vector<char> data_type;
		///
		char at(int x, int y) const;
		///
		char & at(int x, int y);

		/// xsize of the painter area
		int xmax_;
		/// ysize of the painter area
		int ymax_;
		/// the image
		data_type data_;
};

#endif
