#ifndef PRECISE_DOUBLE_H
#define PRECISE_DOUBLE_H

// Define USE_BOOST_MULTIPRECISION externally to enable Boost
#ifdef USE_BOOST_MULTIPRECISION

  // Try to include Boost.Multiprecision
  #include <boost/version.hpp>
  #if !defined(BOOST_VERSION)
    #error "USE_BOOST_MULTIPRECISION defined, but Boost is not available or not found."
  #endif

  #include <boost/multiprecision/cpp_bin_float.hpp>
//  #include <boost/multiprecision/float128.hpp>
  #include <iostream>
  #include <iomanip>

  namespace Precise {
      using pdouble = boost::multiprecision::cpp_bin_float_quad;
      constexpr const char* implementation = "Boost.Multiprecision: cpp_bin_float_quad";

//      using pdouble = boost::multiprecision::float128;
//      constexpr const char* implementation = "Boost.Multiprecision: float_128";

   // High-precision print helper
   //   inline void print(const pdouble& x, const char* label = "value", int digits = 36) {
   //       std::cout << std::scientific << std::setprecision(digits);
   //       std::cout << label << " = " << x << "\n";
   // }
  }

#else

  // Fallback to long double (available on Linux implementations typically with 80-bits)
  namespace Precise {
      using pdouble = long double;
      constexpr const char* implementation = "long double";
  }

#endif

#endif // PRECISE_DOUBLE_H
