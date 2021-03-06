#ifndef CLASS_UTILITIES_H
#define CLASS_UTILITIES_H

#include <iostream>

class Printable {
public:
  virtual ~Printable(){};
  virtual void printItself(std::ostream &os) const noexcept;
};

inline std::ostream &operator<<(std::ostream &os, const Printable &printable) {
  printable.printItself(os);
  return os;
}

#endif