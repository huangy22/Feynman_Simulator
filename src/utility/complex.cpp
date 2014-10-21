//---------------------------------------------------------------------------
// �Nikolai V. Shokhirev, 2004-2008  <nikolai@shokhirev.com>
// http://www.shokhirev.com/nikolai.html
// Variant 2 - based on compound assignment
//---------------------------------------------------------------------------
#include "Complex.h"

using namespace std;

Complex::Complex()
{
    Re = 0.0;
    Im = 0.0;
}

Complex::Complex(real re, real im)
{
    Re = re;
    Im = im;
}

Complex::~Complex()
{
}

Complex::Complex(const Complex &c)
{
    Re = c.Re;
    Im = c.Im;
}

Complex &Complex::operator=(const Complex &c)
{
    if (this != &c) {
        Re = c.Re;
        Im = c.Im;
    }
    return (*this);
}

Complex &Complex::operator=(const real &d)
{
    Re = d;
    Im = 0.0;
    return (*this);
}

Complex &Complex::operator+=(const Complex &c)
{
    Re += c.Re;
    Im += c.Im;
    return (*this);
}

Complex &Complex::operator+=(const real &d)
{
    Re += d;
    return (*this);
}

Complex &Complex::operator-=(const Complex &c)
{
    Re -= c.Re;
    Im -= c.Im;
    return (*this);
}

Complex &Complex::operator-=(const real &d)
{
    Re -= d;
    return (*this);
}

Complex &Complex::operator*=(const Complex &c)
{
    real re = Re;    // backup Re, as we're going to modify it
    real cre = c.Re; // backup c.Re too, in case c *= c;
    Re = Re * c.Re - Im * c.Im;
    Im = re * c.Im + Im * cre;
    return (*this);
}

Complex &Complex::operator*=(const real &d)
{
    Re *= d; // same as above with c.Im = 0 and c.Re = d
    Im *= d; // logically and happily simplified
    return (*this);
}

Complex &Complex::operator/=(const Complex &c)
{
    if (this == &c) { // c /= c;
        Re = 1.0;
        Im = 0.0;
    }
    else {
        real re = Re; // backup Re, as we're going to modify it
        real m = mod2(c);
        Re = (Re * c.Re + Im * c.Im) / m;
        Im = (Im * c.Re - re * c.Im) / m;
    }
    return (*this);
}

Complex &Complex::operator/=(const real &d)
{
    Re /= d;
    Im /= d;
    return (*this);
}

bool IsZero(const Complex &c)
{
    return (c.Re == 0.0) && (c.Im == 0.0);
}

// Prints out a complex with the form (a,b)
ostream &operator<<(ostream &s, const Complex &c)
{
    s << "(" << c.Re << "," << c.Im << ")";
    return s;
}

// Reads a complex number c into the input stream s.
// Format: (a,b)
// spaces can be used between the elements, but not inside an element ("(- 7,2)"
// is incorrect)
// If bad input is encountered, s.setstate(ios::failbit) is called.
istream &operator>>(istream &s, Complex &c)
{
    char left, right, comma;
    if (!((s >> left >> c.Re >> comma >> c.Im >> right) &&
          (left == '(' && right == ')' && comma == ','))) {
        s.setstate(ios::failbit);
    }
    return s;
}

Complex operator+(const Complex &lhs, const Complex &rhs)
{
    return Complex(lhs) += rhs;
}

Complex operator+(real lhs, const Complex &rhs)
{
    return Complex(rhs.Re+lhs,rhs.Im+lhs);
}

Complex operator+(const Complex &lhs, real rhs)
{
    return Complex(lhs.Re+rhs,lhs.Im+rhs);
}

Complex operator+(const Complex &lhs)
{
    return Complex(lhs);
}

Complex operator-(const Complex &lhs, const Complex &rhs)
{
    return Complex(lhs) -= rhs;
}

Complex operator-(real lhs, const Complex &rhs)
{
    return Complex(rhs.Re-lhs,rhs.Im-lhs);
}

Complex operator-(const Complex &lhs, real rhs)
{
    return Complex(lhs.Re+rhs,lhs.Im+rhs);
}

Complex operator-(const Complex &c)
{
    return Complex(-c.Re, -c.Im);
}

Complex operator*(const Complex &lhs, const Complex &rhs)
{
    return Complex(lhs) *= rhs;
}

Complex operator*(real lhs, const Complex &rhs)
{
    return Complex(rhs.Re*lhs,rhs.Im*lhs);
}

Complex operator*(const Complex &lhs, real rhs)
{
    return Complex(lhs.Re*rhs,lhs.Im*rhs);
}

Complex operator/(const Complex &lhs, const Complex &rhs)
{
    return Complex(lhs) /= rhs;
}

Complex operator/(real x, const Complex &y)
{
  double factor;
  factor=1.0/(y.Re*y.Re+y.Im*y.Im);
  return Complex(x*y.Re*factor,-x*y.Im*factor);
}

Complex operator/(const Complex &rhs, real lhs)
{
    return Complex(rhs) /= lhs;
}


// Complex library

real mod2(const Complex &c)
{
    return (c.Re * c.Re + c.Im * c.Im);
}

real mod(const Complex &c)
{
    return sqrt(mod2(c));
}

Complex phase(const Complex &c)
{
    return c/mod(c);
}


bool Equal(Complex c1, Complex c2, real eps)

{

    return (fabs(c1.Re - c2.Re) < eps && fabs(c1.Im - c2.Im) < eps);
}

bool Equal(Complex c1, real r, real i, real eps)

{

    return (fabs(c1.Re - r) < eps && fabs(c1.Im - i) < eps);
}

Complex exp(const Complex &c)
{
    return (exp(c.Re) * Complex(cos(c.Im), sin(c.Im)));
}

// result = sqrt(z), result.Re > 0
Complex Csqrt(Complex z)
{
    Complex result;
    result.Re = sqrt(abs(0.5 * (mod(z) + z.Re)));
    result.Im = sqrt(abs(0.5 * (mod(z) - z.Re)));
    return result;
};

string ToString(Complex c)
{
    string s = ToString(c.Re);
    if (c.Im > 0.0)
        s += "+" + ToString(c.Im) + "j";
    else if (c.Im < 0.0)
        s += ToString(c.Im) + "j";

    return s;
}
