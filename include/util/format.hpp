#ifndef UTIL_ASSERT_HPP
#define UTIL_ASSERT_HPP

// Includes

#include <iostream>
#include <sstream>

// Format namespace

namespace fmt {
   using namespace std::string_literals;
   using namespace std::string_view_literals;

   // String conversion functions

   template<typename T>
   std::string string(const T& value) {
      std::stringstream s;
      s << std::boolalpha << value;
      return s.str();
   }

   // Format functions

   template<typename... Args>
   std::string format(const char* base, const Args&... args) {
      std::string result = base;

      size_t pos = 0;
      (( pos = result.find("{}", pos),
         pos != std::string::npos
            ? result = result.replace(pos, 2, fmt::string(args))
            : result
      ), ...);
      return result;
   }

   // Print functions

   template<typename... Args>
   void printf(const char* base, const Args&... args) {
      std::cout << fmt::format(base, args...) << '\n';
   }

   template<typename... Args>
   void print(const Args&... args) {
      ((std::cout << std::boolalpha << args << ' '), ...);
      std::cout << '\n';
   }

   // Error macros

   #undef assert
   #define warn(base, ...) warnImpl(__FILE__, __LINE__, base, __VA_ARGS__)
   #define raise(base, ...) raiseImpl(__FILE__, __LINE__, base, __VA_ARGS__)
   #define check(condition, base, ...) checkImpl(__FILE__, __LINE__, condition, base, __VA_ARGS__)
   #define assert(condition, base, ...) assertImpl(__FILE__, __LINE__, condition, base, __VA_ARGS__)

   // Private error macro implementations

   template<typename... Args>
   void warnImpl(const char* file, int line, const char* base, const Args&... args) {
      std::cout << "WARNING: '" << file << ":" << line << "': " << fmt::format(base, args...) << '\n';
   }

   template<typename... Args>
   [[noreturn]] void raiseImpl(const char* file, int line, const char* base, const Args&... args) {
      std::cout << "ERROR: '" << file << ":" << line << "': " << fmt::format(base, args...) << '\n';
      std::exit(-1);
   }

   template<typename... Args>
   void checkImpl(const char* file, int line, bool condition, const char* base, const Args&... args) {
      if (not condition) {
         fmt::warnImpl(file, line, base, args...);
      }
   }

   template<typename... Args>
   void assertImpl(const char* file, int line, bool condition, const char* base, const Args&... args) {
      if (not condition) {
         fmt::raiseImpl(file, line, base, args...);
      }
   }
}

#endif
