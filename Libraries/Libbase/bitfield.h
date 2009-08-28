#ifndef __bitfield_h
#define __bitfield_h

#include "config.h"
#include "vector.h"

#include <iostream>
#include <string>

namespace libbase {

/*!
 * \brief   Bitfield (register of a set size).
 * \author  Johann Briffa
 *
 * \section svn Version Control
 * - $Revision$
 * - $Date$
 * - $Author$
 */

class bitfield {
   static int defsize; //!< default size
   // member variables
   int32u field; //!< bit field value
   int bits; //!< number of bits
private:
   int32u mask() const;
   void check_range(int32u f) const;
   static void check_fieldsize(int b);
   void init(const char *s);
   // Partial extraction and indexed access
   bitfield extract(const int hi, const int lo) const;
   bitfield extract(const int b) const;
public:
   /*! \name Constructors / Destructors */
   bitfield();
   explicit bitfield(const char *s)
      {
      init(s);
      }
   bitfield(const int32u field, const int bits);
   explicit bitfield(const vector<bool>& v);
   // @}

   /*! \name Type conversion */
   operator int32u() const
      {
      return field;
      }
   std::string asstring() const;
   vector<bool> asvector() const;
   // @}

   // Field size methods
   int size() const
      {
      return bits;
      }
   void resize(const int b);
   static void setdefsize(const int b);

   // Copy and Assignment
   bitfield& operator=(const bitfield& x);
   bitfield& operator=(const int32u x);

   // Partial extraction and indexed access
   bitfield operator()(const int hi, const int lo) const
      {
      return extract(hi, lo);
      }
   bitfield operator()(const int b) const
      {
      return extract(b);
      }

   // Bit-reversal method
   bitfield reverse() const;

   // Logical operators - OR, AND, XOR
   bitfield operator|(const bitfield& x) const;
   bitfield operator&(const bitfield& x) const;
   bitfield operator^(const bitfield& x) const;
   bitfield& operator|=(const bitfield& x);
   bitfield& operator&=(const bitfield& x);
   bitfield& operator^=(const bitfield& x);

   // Convolution operator
   bitfield operator*(const bitfield& x) const;
   // Concatenation operator
   bitfield operator+(const bitfield& x) const;
   // Shift-register operators - sequence shift-in
   bitfield operator<<(const bitfield& x) const;
   bitfield operator>>(const bitfield& x) const;
   // Shift-register operators - zero shift-in
   bitfield operator<<(const int x) const;
   bitfield operator>>(const int x) const;
   bitfield& operator<<=(const int x);
   bitfield& operator>>=(const int x);

   // Stream I/O
   friend std::ostream& operator<<(std::ostream& s, const bitfield& b);
   friend std::istream& operator>>(std::istream& s, bitfield& b);
};

} // end namespace

#endif