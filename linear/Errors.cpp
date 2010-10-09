/**
 * @file    Errors.cpp
 * @brief   Factor Graph Values
 * @brief   Errors
 * @author  Carlos Nieto
 * @author  Christian Potthast
 */

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <gtsam/linear/Errors.h>

using namespace std;

namespace gtsam {

/* ************************************************************************* */
void Errors::print(const std::string& s) const {
  odprintf("%s:\n", s.c_str());
  BOOST_FOREACH(const Vector& v, *this)
    gtsam::print(v);
}

/* ************************************************************************* */
struct equalsVector : public std::binary_function<const Vector&, const Vector&, bool> {
	double tol_;
	equalsVector(double tol = 1e-9) : tol_(tol) {}
	bool operator()(const Vector& expected, const Vector& actual) {
		return equal_with_abs_tol(expected, actual,tol_);
	}
};

bool Errors::equals(const Errors& expected, double tol) const {
	if( size() != expected.size() ) return false;
	return equal(begin(),end(),expected.begin(),equalsVector(tol));
	// TODO: use boost::bind(&equal_with_abs_tol,_1, _2,tol)
}

/* ************************************************************************* */
Errors Errors::operator-(const Errors& b) const {
#ifndef NDEBUG
	size_t m = size();
	if (b.size()!=m)
    throw(std::invalid_argument("Errors::operator-: incompatible sizes"));
#endif
	Errors result;
	Errors::const_iterator it = b.begin();
  BOOST_FOREACH(const Vector& ai, *this)
		result.push_back(ai - *(it++));
	return result;
}

/* ************************************************************************* */
double dot(const Errors& a, const Errors& b) {
#ifndef NDEBUG
	size_t m = a.size();
  if (b.size()!=m)
    throw(std::invalid_argument("Errors::dot: incompatible sizes"));
#endif
  double result = 0.0;
	Errors::const_iterator it = b.begin();
  BOOST_FOREACH(const Vector& ai, a)
		result += gtsam::dot(ai, *(it++));
	return result;
}

/* ************************************************************************* */
void axpy(double alpha, const Errors& x, Errors& y) {
	Errors::const_iterator it = x.begin();
  BOOST_FOREACH(Vector& yi, y)
		axpy(alpha,*(it++),yi);
}

/* ************************************************************************* */
void print(const Errors& a, const string& s) {
	a.print(s);
}

/* ************************************************************************* */

} // gtsam
