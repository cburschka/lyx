#ifndef TEXTPAINTER_H
#define TEXTPAINTER_H

#include <vector>

class TextPainter {
	public:
		///
		TextPainter(int xmax, int ymax);
		///
		void draw(int x, int y, char const * str);

	private:
		///
		typedef std::vector<char> data_type;
		///
		char at(int x, int y) const;
		///
		char & at(int x, int y);

		///
		data_type data_;
		///
		int xmax_;
		///
		int ymax_;
};

#endif
