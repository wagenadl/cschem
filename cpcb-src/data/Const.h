// Const.h

#ifndef CONST_H

#define CONST_H

template<class X> X const &as_const(X &x) { return const_cast<X const &>(x); }
template<class X> X &as_nonconst(X const &x) { return const_cast<X &>(x); }

#endif
