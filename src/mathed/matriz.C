#include <config.h>

#include <cstring>

#include "matriz.h"

static inline
int odd(int x) { return ((x) & 1); }

#define mateq(m1, m2)  memcpy(m1, m2, sizeof(matriz_data))


Matriz::matriz_data const Matriz::MATIDEN = { {1, 0}, {0, 1}};


Matriz::Matriz()
{
	mateq(m_, MATIDEN);
}


void Matriz::rota(int code)
{
	matriz_data r;
	mateq(r, MATIDEN);
	float const cs = (odd(code)) ? 0 : (1 - code);
	float const sn = (odd(code)) ? (2 - code) : 0;
	r[0][0] = cs;         r[0][1] = sn;
	r[1][0] = -r[0][1];   r[1][1] = r[0][0];
	matmat(r);
}


void Matriz::escala(float x, float y)
{
	matriz_data s;
	mateq(s, MATIDEN);
	s[0][0] = x;  s[1][1] = y;
	matmat(s);
}


void Matriz::matmat(matriz_data & a)
{
	matriz_data c;   
	for (int i = 0; i < 2; ++i) {
		c[0][i] = a[0][0] * m_[0][i] + a[0][1] * m_[1][i];
		c[1][i] = a[1][0] * m_[0][i] + a[1][1] * m_[1][i];
	}
	mateq(m_, c);
}


void Matriz::transf(float xp, float yp, float & x, float & y)
{
	x = m_[0][0] * xp + m_[0][1] * yp;
	y = m_[1][0] * xp + m_[1][1] * yp;
}
