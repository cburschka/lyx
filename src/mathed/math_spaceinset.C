#include <config.h>

#include "math_spaceinset.h"
#include "LColor.h"
#include "Painter.h"
#include "mathed/support.h"
#include "support/LOstream.h"

using std::ostream;


void MathSpaceInset::Metrics()
{
	width = space ? space * 2 : 2;
	if (space > 3) width *= 2;
	if (space == 5) width *= 2;
	width += 4;
	ascent = 4; descent = 0;
}


void MathSpaceInset::SetSpace(int sp)
{ 
	space = sp;
	Metrics();
}


MathSpaceInset::MathSpaceInset(int sp, short ot, short st)
	: MathedInset("", ot, st), space(sp)
{}


MathedInset * MathSpaceInset::Clone()
{
	return new MathSpaceInset(space, GetType(), GetStyle());
}


void
MathSpaceInset::draw(Painter & pain, int x, int y)
{ 
	
// XPoint p[4] = {{++x, y-3}, {x, y}, {x+width-2, y}, {x+width-2, y-3}};
	
// Sadly, HP-UX CC can't handle that kind of initialization.
	
	int xp[4];
	int yp[4];
	
	xp[0] = ++x;            yp[0] = y - 3;
	xp[1] = x;	           yp[1] = y;
	xp[2] = x + width - 2;  yp[2] = y;
	xp[3] = x + width - 2;  yp[3] = y - 3;
	
	pain.lines(xp, yp, 4, (space) ? LColor::latex : LColor::math);
}


void
MathSpaceInset::Write(ostream & os, bool /* fragile */)
{
   if (space >= 0 && space < 6) {
	   os << '\\' << latex_mathspace[space] << ' ';
   }
}


int MathSpaceInset::GetSpace()
{
	return space;
}
