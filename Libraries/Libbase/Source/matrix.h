#ifndef __matrix_h
#define __matrix_h

#include "config.h"
#include "size.h"
#include "vector.h"
#include <stdlib.h>
#include <iostream>
#include <algorithm>

namespace libbase {

/* \note
   To comply with the standard, matrix's friend functions must be declared
   before the main class. Consequently, a declaration of the class itself
   is also required before that.
*/
template <class T>
class matrix;
template <class T>
class masked_matrix;

template <class T>
std::ostream& operator<<(std::ostream& s, const matrix<T>& x);
template <class T>
std::istream& operator>>(std::istream& s, matrix<T>& x);

/*!
   \brief   Size specialization for matrix.
   \author  Johann Briffa

   \section svn Version Control
   - $Revision$
   - $Date$
   - $Author$
*/

template <>
class size_type<matrix> {
private:
   int  m;  //!< Number of rows
   int  n;  //!< Number of columns
public:
   /*! \brief Principal Constructor
   */
   explicit size_type(int m=0, int n=0) { this->m = m; this->n = n; };
   /*! \brief Conversion to integer
      Returns the total number of elements
   */
   operator int() const { return m*n; };
   /*! \brief Comparison of two size objects
      Only true if both dimensions are the same
   */
   bool operator==(const size_type<matrix>& rhs) const { return (m==rhs.m) && (n==rhs.n); };
   /*! \brief Number of rows (first index) */
   int rows() const { return m; };
   /*! \brief Number of columns (second index) */
   int cols() const { return n; };
   /*! \brief Stream output */
   friend std::ostream& operator<<(std::ostream& sout, const size_type<matrix> r)
      {
      sout << r.m << '\t' << r.n;
      return sout;
      };
   /*! \brief Stream input */
   friend std::istream& operator>>(std::istream& sin, size_type<matrix>& r)
      {
      sin >> r.m >> r.n;
      return sin;
      }
};

/*!
   \brief   Generic 2D Matrix.
   \author  Johann Briffa

   \section svn Version Control
   - $Revision$
   - $Date$
   - $Author$

   Arithmetic functions are part of the matrix class. This includes arithmetic
   operations between matrices, constant matrix initialisation routines, and
   some statistical functions.

   This class follows the usual mathematical convention, where the first index
   represents the row and second represents the column. This is consistent with
   Matlab notation.

   \note Empty matrices (that is, ones with no elements) are defined and valid.

   \note Range-checking and other validation functions are only operative in
         debug mode.


   \todo Extract common implementation of copy assignment operators
*/

template <class T>
class matrix {
   friend class masked_matrix<T>;
private:
   size_type<libbase::matrix>   m_size;
   T    **m_data;
protected:
   /*! \name Memory allocation functions */
   void alloc(const int m, const int n);
   void free();
   // @}
public:
   /*! \name Constructors / destructors */
   /*! \brief Default constructor
      This exists instead of default-values in principal constructor to avoid
      allowing the situation where a constructor is given a _single_ integer
      parameter (i.e. x has a value, y takes its default)
   */
   matrix() { alloc(0,0); };
   /*! \brief Principal constructor
      \note Does not initialize elements.
   */
   matrix(const int m, const int n) { alloc(m,n); };
   /*! \brief Principal constructor - alternative form
      \note Does not initialize elements.
   */
   matrix(const size_type<libbase::matrix>& size) { alloc(size.rows(),size.cols()); };
   /*! \brief Copy constructor
   */
   matrix(const matrix<T>& x);
   ~matrix() { free(); };
   // @}

   /*! \name Resizing operations */
   void init(const int m, const int n);
   /*! \copydoc init()
      This overload takes a matrix-size object as argument.
   */
   void init(const size_type<libbase::matrix>& size) { init(size.rows(), size.cols()); };
   /*! \copydoc init()
      This overload takes another matrix as argument, to allow easier (and
      neater) sizing of one matrix based on another. This is a template
      function to allow the argument matrix to be of a different type.
      \todo Consider removing this overload, as it is redundant
   */
   template <class A>
   void init(const matrix<A>& x) { init(x.size()); };
   // @}

   /*! \name Matrix copy, vector conversion, and value initialisation */
   matrix<T>& copyfrom(const matrix<T>& x);
   matrix<T>& operator=(const matrix<T>& x);
   template <class A>
   matrix<T>& operator=(const matrix<A>& x);
   template <class A>
   matrix<T>& operator=(const vector<A>& x);
   matrix<T>& operator=(const T x);
   operator vector<T>() const;
   // @}

   /*! \name Insert/extract rows/columns as vectors */
   void insertrow(const vector<T>& v, const int i);
   void insertcol(const vector<T>& v, const int j);
   void extractrow(vector<T>& v, const int i) const;
   void extractcol(vector<T>& v, const int j) const;
   vector<T> extractrow(const int i) const;
   vector<T> extractcol(const int j) const;
   // @}

   /*! \name Bind a mask to a matrix */
   masked_matrix<T> mask(const matrix<bool>& x)
      { return masked_matrix<T>(this, x); };
   // @}

   /*! \name Index operators (perform boundary checking) */
   T& operator()(const int i, const int j);
   T operator()(const int i, const int j) const;
   // @}

   /*! \name Information functions */
   //! Total number of elements
   size_type<libbase::matrix> size() const { return m_size; };
   // @}

   /*! \name Serialization and stream input & output */
   void serialize(std::ostream& s) const;
   void serialize(std::ostream& s, char spacer) const
      { serialize(s); s << spacer; };
   void serialize(std::istream& s);
   friend std::ostream& operator<< <>(std::ostream& s, const matrix<T>& x);
   friend std::istream& operator>> <>(std::istream& s, matrix<T>& x);
   // @}

   /*! \name Comparison (mask-creation) operations */
   matrix<bool> operator==(const matrix<T>& x) const;
   matrix<bool> operator!=(const matrix<T>& x) const;
   matrix<bool> operator<=(const matrix<T>& x) const;
   matrix<bool> operator>=(const matrix<T>& x) const;
   matrix<bool> operator<(const matrix<T>& x) const;
   matrix<bool> operator>(const matrix<T>& x) const;
   matrix<bool> operator==(const T x) const;
   matrix<bool> operator!=(const T x) const;
   matrix<bool> operator<=(const T x) const;
   matrix<bool> operator>=(const T x) const;
   matrix<bool> operator<(const T x) const;
   matrix<bool> operator>(const T x) const;
   // @}

   /*! \name Direct comparison operations */
   bool isequalto(const matrix<T>& x) const;
   bool isnotequalto(const matrix<T>& x) const;
   // @}

   /*! \name Arithmetic operations - unary */
   matrix<T>& operator+=(const matrix<T>& x);
   matrix<T>& operator-=(const matrix<T>& x);
   matrix<T>& operator*=(const matrix<T>& x);
   matrix<T>& operator/=(const matrix<T>& x);
   matrix<T>& multiplyby(const matrix<T>& x);
   matrix<T>& divideby(const matrix<T>& x);
   matrix<T>& operator+=(const T x);
   matrix<T>& operator-=(const T x);
   matrix<T>& operator*=(const T x);
   matrix<T>& operator/=(const T x);
   // @}

   /*! \name Arithmetic operations - binary */
   matrix<T> operator+(const matrix<T>& x) const;
   matrix<T> operator-(const matrix<T>& x) const;
   matrix<T> operator*(const matrix<T>& x) const;
   vector<T> operator*(const vector<T>& x) const;
   matrix<T> operator/(const matrix<T>& x) const;
   matrix<T> multiply(const matrix<T>& x) const;
   matrix<T> divide(const matrix<T>& x) const;
   matrix<T> operator+(const T x) const;
   matrix<T> operator-(const T x) const;
   matrix<T> operator*(const T x) const;
   matrix<T> operator/(const T x) const;
   // @}

   /*! \name Boolean operations - modifying */
   matrix<T>& operator&=(const matrix<T>& x);
   matrix<T>& operator|=(const matrix<T>& x);
   matrix<T>& operator^=(const matrix<T>& x);
   // @}

   /*! \name Boolean operations - non-modifying */
   matrix<T> operator!() const;
   matrix<T> operator&(const matrix<T>& x) const;
   matrix<T> operator|(const matrix<T>& x) const;
   matrix<T> operator^(const matrix<T>& x) const;
   // @}

   /*! \name User-defined operations */
   matrix<T>& apply(T f(T));
   // @}

   /*! \name Matrix-arithmetic operations */
   matrix<T> inverse() const;
   matrix<T> transpose() const;
   // @}

   /*! \name Statistical operations */
   T min() const;
   T max() const;
   T sum() const;
   T sumsq() const;
   //! Compute the mean value of matrix elements
   T mean() const { return sum()/T(size()); };
   T var() const;
   //! Compute standard deviation of matrix elements
   T sigma() const { return sqrt(var()); };
   // @}

   /*! \name Static functions */
   static matrix<T> eye(int n);
   // @}
};


//! If there is memory allocated, free it
template <class T>
inline void matrix<T>::free()
   {
   if(m_size > 0)
      {
      for(int i=0; i<m_size.rows(); i++)
         delete[] m_data[i];
      delete[] m_data;
      }
   }

/*! \brief Allocates memory for (x,y) elements and updates sizes
   \note Detects invalid size values (either x or y being 0, but not both)
*/
template <class T>
inline void matrix<T>::alloc(const int x, const int y)
   {
   if(x==0 && y==0)
      {
      m_size = size_type<libbase::matrix>(0,0);
      m_data = NULL;
      }
   else
      {
      assertalways(x>0 && y>0);
      m_size = size_type<libbase::matrix>(x,y);
      typedef T* Tp;
      m_data = new Tp[x];
      for(int i=0; i<x; i++)
         m_data[i] = new T[y];
      }
   }

// constructor / destructor functions

template <class T>
inline matrix<T>::matrix(const matrix<T>& x)
   {
   alloc(x.m_size.rows(), x.m_size.cols());
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] = x.m_data[i][j];
   }

/*! \brief Set matrix to given size, freeing if and as required

   This method leaves the matrix as it is if the size was already correct, and
   frees/reallocates if necessary. This helps reduce redundant free/alloc
   operations on matrices which keep the same size.
*/
template <class T>
inline void matrix<T>::init(const int m, const int n)
   {
   if(m==m_size.rows() && n==m_size.cols())
      return;
   free();
   alloc(m,n);
   }

// matrix copy and value initialisation

/*! \brief Copies data from another matrix without resizing this one

   Adds support for working with different-sized matrices (in place of
   resizing operations which would be quite expensive).

   \note Opted for this rather than changing the definition of operator=
         because it's convenient for '=' to copy _everything_ from the source
         to the destination; otherwise we would land into obscure problems in
         some cases (like when we're trying to copy a vector of matrices).
*/
template <class T>
inline matrix<T>& matrix<T>::copyfrom(const matrix<T>& x)
   {
   const int rows = std::min(m_size.rows(), x.m_size.rows());
   const int cols = std::min(m_size.cols(), x.m_size.cols());
   for(int i=0; i<rows; i++)
      for(int j=0; j<cols; j++)
         m_data[i][j] = x.m_data[i][j];
   return *this;
   }

template <class T>
inline matrix<T>& matrix<T>::operator=(const matrix<T>& x)
   {
   init(x.size());
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] = x(i,j);
   return *this;
   }

template <class T>
template <class A>
inline matrix<T>& matrix<T>::operator=(const matrix<A>& x)
   {
   init(x.size());
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] = x(i,j);
   return *this;
   }

template <class T>
template <class A>
inline matrix<T>& matrix<T>::operator=(const vector<A>& x)
   {
   init(x.size(), 1);
   for(int i=0; i<m_size.rows(); i++)
      m_data[i][0] = x(i);
   return *this;
   }

template <class T>
inline matrix<T>& matrix<T>::operator=(const T x)
   {
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] = x;
   return *this;
   }

/*! \brief Convert matrix to a vector
   Elements are extracted in column-major order (ie. starting with the top-left
   first go down then across).
*/
template <class T>
inline matrix<T>::operator vector<T>() const
   {
   vector<T> v(size());
   int k=0;
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         v(k++) = m_data[i][j];
   return v;
   }

// insert/extract rows/columns as vectors

/*! \brief Insert vector into row 'i'
*/
template <class T>
inline void matrix<T>::insertrow(const vector<T>& v, const int i)
   {
   assert(v.size() == m_size.cols());
   for(int j=0; j<m_size.cols(); j++)
      m_data[i][j] = v(j);
   }

/*! \brief Insert vector into column 'j'
*/
template <class T>
inline void matrix<T>::insertcol(const vector<T>& v, const int j)
   {
   assert(v.size() == m_size.rows());
   for(int i=0; i<m_size.rows(); i++)
      m_data[i][j] = v(i);
   }

/*! \brief Extract row 'i' as a vector
   The target vector needs to be passed as a parameter; the expression format
   can be improved aesthetically, however the present format clearly
   communicates what is happening.
*/
template <class T>
inline void matrix<T>::extractrow(vector<T>& v, const int i) const
   {
   assert(i>=0 && i<m_size.rows());
   v.init(m_size.cols());
   for(int j=0; j<m_size.cols(); j++)
      v(j) = m_data[i][j];
   }

/*! \brief Extract column 'j' as a vector
   The target vector needs to be passed as a parameter; the expression format
   can be improved aesthetically, however the present format clearly
   communicates what is happening.
*/
template <class T>
inline void matrix<T>::extractcol(vector<T>& v, const int j) const
   {
   assert(j>=0 && j<m_size.cols());
   v.init(m_size.rows());
   for(int i=0; i<m_size.rows(); i++)
      v(i) = m_data[i][j];
   }

/*! \brief Extract row 'i' as a vector
*/
template <class T>
inline vector<T> matrix<T>::extractrow(const int i) const
   {
   vector<T> v;
   extractrow(v,i);
   return v;
   }

/*! \brief Extract column 'j' as a vector
*/
template <class T>
inline vector<T> matrix<T>::extractcol(const int j) const
   {
   vector<T> v;
   extractcol(v,j);
   return v;
   }

// index operators (perform boundary checking)

template <class T>
inline T& matrix<T>::operator()(const int i, const int j)
   {
   assert(i>=0 && i<m_size.rows());
   assert(j>=0 && j<m_size.cols());
   return m_data[i][j];
   }

template <class T>
inline T matrix<T>::operator()(const int i, const int j) const
   {
   assert(i>=0 && i<m_size.rows());
   assert(j>=0 && j<m_size.cols());
   return m_data[i][j];
   }

/*! \brief Writes matrix data to output stream.
   This function is intended for interfacing with file formats that do not use
   the serialization format of this class.

   \todo Transpose output to conform with convention
*/
template <class T>
inline void matrix<T>::serialize(std::ostream& s) const
   {
   for(int j=0; j<m_size.cols(); j++)
      {
      s << m_data[0][j];
      for(int i=1; i<m_size.rows(); i++)
         s << "\t" << m_data[i][j];
      s << "\n";
      }
   }

/*! \brief Reads matrix data from input stream.
   This function is intended for interfacing with file formats that do not use
   the serialization format of this class.

   \note Assumes that the current matrix already has the correct size.

   \todo Transpose output to conform with convention
*/
template <class T>
inline void matrix<T>::serialize(std::istream& s)
   {
   for(int j=0; j<m_size.cols(); j++)
      for(int i=0; i<m_size.rows(); i++)
         s >> m_data[i][j];
   }

/*! \brief Writes matrix to output stream.
   Includes matrix size, to allow correct reconstruction when reading in.
*/
template <class T>
inline std::ostream& operator<<(std::ostream& s, const matrix<T>& x)
   {
   s << x.m_size << "\n";
   x.serialize(s);
   return s;
   }

/*! \brief Reads and reconstructs matrix from input stream.
*/
template <class T>
inline std::istream& operator>>(std::istream& s, matrix<T>& x)
   {
   size_type<matrix> size;
   s >> size;
   x.init(size);
   x.serialize(s);
   return s;
   }

// comparison (mask-creation) operations

template <class T>
inline matrix<bool> matrix<T>::operator==(const matrix<T>& x) const
   {
   assert(x.m_size == m_size);
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] == x.m_data[i][j]);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator!=(const matrix<T>& x) const
   {
   assert(x.m_size == m_size);
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] != x.m_data[i][j]);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator<=(const matrix<T>& x) const
   {
   assert(x.m_size == m_size);
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] <= x.m_data[i][j]);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator>=(const matrix<T>& x) const
   {
   assert(x.m_size == m_size);
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] >= x.m_data[i][j]);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator<(const matrix<T>& x) const
   {
   assert(x.m_size == m_size);
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] < x.m_data[i][j]);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator>(const matrix<T>& x) const
   {
   assert(x.m_size == m_size);
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] > x.m_data[i][j]);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator==(const T x) const
   {
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] == x);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator!=(const T x) const
   {
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] != x);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator<=(const T x) const
   {
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] <= x);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator>=(const T x) const
   {
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] >= x);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator<(const T x) const
   {
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] < x);
   return r;
   }

template <class T>
inline matrix<bool> matrix<T>::operator>(const T x) const
   {
   matrix<bool> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r(i,j) = (m_data[i][j] > x);
   return r;
   }

// comparison (mask-creation) operations

template <class T>
inline bool matrix<T>::isequalto(const matrix<T>& x) const
   {
   if(x.m_size != m_size)
      return false;
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         if(m_data[i][j] != x.m_data[i][j])
            return false;
   return true;
   }

template <class T>
inline bool matrix<T>::isnotequalto(const matrix<T>& x) const
   {
   return !isequalto(x);
   }

// arithmetic operations - unary

template <class T>
inline matrix<T>& matrix<T>::operator+=(const matrix<T>& x)
   {
   assert(x.m_size == m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] += x.m_data[i][j];
   return *this;
   }

template <class T>
inline matrix<T>& matrix<T>::operator-=(const matrix<T>& x)
   {
   assert(x.m_size == m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] -= x.m_data[i][j];
   return *this;
   }

/*!
   \brief Ordinary matrix multiplication
   \param  x   Matrix to be multiplied to this one
   \return The updated (multiplied-into) matrix
*/
template <class T>
inline matrix<T>& matrix<T>::operator*=(const matrix<T>& x)
   {
   matrix<T> r = *this * x;
   return *this = r;
   }

/*!
   \brief Ordinary matrix division
   \param  x   Matrix to divide this one by
   \return The updated (divided-by) matrix
*/
template <class T>
inline matrix<T>& matrix<T>::operator/=(const matrix<T>& x)
   {
   matrix<T> r = *this / x;
   return *this = r;
   }

/*!
   \brief Array multiplication (element-by-element) of matrices
   \param  x   Matrix to be multiplied to this one
   \return The updated (multiplied-into) matrix
   \note For A.*B, the size of A must be the same as the size of B.
*/
template <class T>
inline matrix<T>& matrix<T>::multiplyby(const matrix<T>& x)
   {
   assert(x.m_size == m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] *= x.m_data[i][j];
   return *this;
   }

/*!
   \brief Array division (element-by-element) of matrices
   \param  x   Matrix to divide this one by
   \return The updated (divided-into) matrix
   \note For A./B, the size of A must be the same as the size of B.
*/
template <class T>
inline matrix<T>& matrix<T>::divideby(const matrix<T>& x)
   {
   assert(x.m_size == m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] /= x.m_data[i][j];
   return *this;
   }

template <class T>
inline matrix<T>& matrix<T>::operator+=(const T x)
   {
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] += x;
   return *this;
   }

template <class T>
inline matrix<T>& matrix<T>::operator-=(const T x)
   {
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] -= x;
   return *this;
   }

template <class T>
inline matrix<T>& matrix<T>::operator*=(const T x)
   {
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] *= x;
   return *this;
   }

template <class T>
inline matrix<T>& matrix<T>::operator/=(const T x)
   {
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] /= x;
   return *this;
   }

// arithmetic operations - binary

template <class T>
inline matrix<T> matrix<T>::operator+(const matrix<T>& x) const
   {
   matrix<T> r = *this;
   r += x;
   return r;
   }

template <class T>
inline matrix<T> matrix<T>::operator-(const matrix<T>& x) const
   {
   matrix<T> r = *this;
   r -= x;
   return r;
   }

/*!
   \brief Ordinary matrix multiplication
   \param  x   Matrix to be multiplied to this one
   \return The result of 'this' multiplied by 'x'
   For matrix multiplication A.B, the number of columns of A must be the same
   as the number of rows of B.
   If A is an m-by-n matrix and B is an n-by-p matrix, then the product is an
   m-by-p matrix, where the elements are given by:
   \f[ AB_{i,j} = \sum_{k=1}^{n} a_{i,k} b_{k,j} \f]
*/
template <class T>
inline matrix<T> matrix<T>::operator*(const matrix<T>& x) const
   {
   assert(m_size.rows() == x.m_size.cols());
   matrix<T> r(x.m_size.rows(),m_size.cols());
   for(int i=0; i<r.m_size.cols(); i++)
      for(int j=0; j<r.m_size.rows(); j++)
         {
         r.m_data[j][i] = 0;
         for(int k=0; k<m_size.rows(); k++)
            r.m_data[j][i] += m_data[k][i] * x.m_data[j][k];
         }
   return r;
   }

/*!
   \brief Ordinary matrix multiplication by column vector
   \param  x   Vector to be multiplied to this matrix
   \return The result of 'this' multiplied by 'x'
   For multiplication A.B, where A is a matrix and B is a vector,
   the number of columns of A must be the same as the number of rows of B.
   If A is an m-by-n matrix and B is an n-by-1 vector, then the product is an
   m-by-1 matrix, where the elements are given by:
   \f[ AB_{i} = \sum_{k=1}^{n} a_{i,k} b_{k} \f]
*/
template <class T>
inline vector<T> matrix<T>::operator*(const vector<T>& x) const
   {
   assert(m_size.rows() == x.size());
   vector<T> r(m_size.cols());
   for(int i=0; i<r.size(); i++)
      {
      r(i) = 0;
      for(int k=0; k<m_size.rows(); k++)
         r(i) += m_data[k][i] * x(k);
      }
   return r;
   }

/*!
   \brief Ordinary matrix division by matrix inversion
   \param  x   Matrix to divide this one by
   \return The result of 'this' divided by 'x'
*/
template <class T>
inline matrix<T> matrix<T>::operator/(const matrix<T>& x) const
   {
   return *this * x.inverse();
   }

/*!
   \brief Array multiplication (element-by-element) of matrices
   \param  x   Matrix to be multiplied to this one
   \return The result of 'this' multiplied by 'x'
   \note For A.*B, the size of A must be the same as the size of B.
*/
template <class T>
inline matrix<T> matrix<T>::multiply(const matrix<T>& x) const
   {
   assert(x.m_size == m_size);
   matrix<T> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r.m_data[i][j] = m_data[i][j] * x.m_data[i][j];
   return r;
   }

/*!
   \brief Array division (element-by-element) of matrices
   \param  x   Matrix to divide this one by
   \return The result of 'this' divided by 'x'
   \note For A./B, the size of A must be the same as the size of B.
*/
template <class T>
inline matrix<T> matrix<T>::divide(const matrix<T>& x) const
   {
   assert(x.m_size == m_size);
   matrix<T> r(m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r.m_data[i][j] = m_data[i][j] / x.m_data[i][j];
   return r;
   }

template <class T>
inline matrix<T> matrix<T>::operator+(const T x) const
   {
   matrix<T> r = *this;
   r += x;
   return r;
   }

template <class T>
inline matrix<T> matrix<T>::operator-(const T x) const
   {
   matrix<T> r = *this;
   r -= x;
   return r;
   }

template <class T>
inline matrix<T> matrix<T>::operator*(const T x) const
   {
   matrix<T> r = *this;
   r *= x;
   return r;
   }

template <class T>
inline matrix<T> matrix<T>::operator/(const T x) const
   {
   matrix<T> r = *this;
   r /= x;
   return r;
   }

// boolean operations - modifying

template <class T>
inline matrix<T>& matrix<T>::operator&=(const matrix<T>& x)
   {
   assert(x.m_size == m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] &= x.m_data[i][j];
   return *this;
   }

template <class T>
inline matrix<T>& matrix<T>::operator|=(const matrix<T>& x)
   {
   assert(x.m_size == m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] |= x.m_data[i][j];
   return *this;
   }

template <class T>
inline matrix<T>& matrix<T>::operator^=(const matrix<T>& x)
   {
   assert(x.m_size == m_size);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] ^= x.m_data[i][j];
   return *this;
   }

// boolean operations - non-modifying

template <class T>
inline matrix<T> matrix<T>::operator!() const
   {
   matrix<T> r(*this);
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r.m_data[i][j] = !m_data[i][j];
   return r;
   }

template <class T>
inline matrix<T> matrix<T>::operator&(const matrix<T>& x) const
   {
   matrix<T> r = *this;
   r &= x;
   return r;
   }

template <class T>
inline matrix<T> matrix<T>::operator|(const matrix<T>& x) const
   {
   matrix<T> r = *this;
   r |= x;
   return r;
   }

template <class T>
inline matrix<T> matrix<T>::operator^(const matrix<T>& x) const
   {
   matrix<T> r = *this;
   r ^= x;
   return r;
   }

/*! \brief Perform user-defined operation on all matrix elements

   \note Removed the instance of apply() whose given function's parameter is
         const, since this was causing problems with gcc on Solaris.
*/
template <class T>
inline matrix<T>& matrix<T>::apply(T f(T))
   {
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         m_data[i][j] = f(m_data[i][j]);
   return *this;
   }

// matrix-arithmetic operations

/*!
   \brief Matrix inversion, by direct Gauss-Jordan elimination
   \return The inverse of this matrix
   \invariant Matrix must be square
   \note Template class must provide the subtraction, division, and
         multiplication operators, as well as conversion to/from integer
   \note Performs row pivoting when necessary.
*/
template <class T>
inline matrix<T> matrix<T>::inverse() const
   {
   assertalways(m_size.rows() == m_size.cols());
   const int n = m_size.rows();
   matrix<T> r = eye(n);
   // create copy of rows of this matrix
   vector< vector<T> > arows(n);
   for(int i=0; i<n; i++)
      extractrow(arows(i), i);
   // create copy of rows of identity
   vector< vector<T> > rrows(n);
   for(int i=0; i<n; i++)
      r.extractrow(rrows(i), i);
   // perform Gauss-Jordan elimination
   // repeat for all rows
   for(int i=0; i<n; i++)
      {
      //trace << "DEBUG (matrix): G-J elimination on row " << i << "\n";
      //trace << "DEBUG (matrix): A = " << arows;
      //trace << "DEBUG (matrix): R = " << rrows;
      // find a suitable pivot element
      if(arows(i)(i) == 0)
         for(int j=i+1; j<n; j++)
            if(arows(j)(i) != 0)
               {
               std::swap(rrows(i),rrows(j));
               std::swap(arows(i),arows(j));
               //trace << "DEBUG (matrix): swapped rows " << i << "<->" << j << "\n";
               //trace << "DEBUG (matrix): A = " << arows;
               //trace << "DEBUG (matrix): R = " << rrows;
               break;
               }
      // divide by pivot element
      rrows(i) /= arows(i)(i);
      arows(i) /= arows(i)(i);
      // subtract the required amount from all other rows
      for(int j=0; j<n; j++)
         {
         if(j == i)
            continue;
         rrows(j) -= rrows(i) * arows(j)(i);
         arows(j) -= arows(i) * arows(j)(i);
         }
      }
   // copy back rows of result
   for(int i=0; i<n; i++)
      r.insertrow(rrows(i), i);
   return r;
   }

/*!
   \brief Matrix transpose
*/
template <class T>
inline matrix<T> matrix<T>::transpose() const
   {
   matrix<T> r(m_size.cols(),m_size.rows());
   // copy over data
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         r.m_data[j][i] = m_data[i][j];
   return r;
   }

// statistical operations

/*! \brief Determines the smallest element in the matrix

   \note This assumes that less-than comparison is defined in operator (<).

   \note This is only valid for non-empty matrices.
*/
template <class T>
inline T matrix<T>::min() const
   {
   assert(m_size > 0);
   T result = m_data[0][0];
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         if(m_data[i][j] < result)
            result = m_data[i][j];
   return result;
   }

/*! \brief Determines the largest element in the matrix

   \note This assumes that greater-than comparison is defined in operator (>).

   \note This is only valid for non-empty matrices.
*/
template <class T>
inline T matrix<T>::max() const
   {
   assert(m_size > 0);
   T result = m_data[0][0];
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         if(m_data[i][j] > result)
            result = m_data[i][j];
   return result;
   }

/*! \brief Computes sum of elements in matrix

   \note This assumes that addition is defined for the type, in the accumulate
         operator (+=). Also, it is assumed that '0' is defined for the type.
*/
template <class T>
inline T matrix<T>::sum() const
   {
   assert(m_size > 0);
   T result = 0;
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         result += m_data[i][j];
   return result;
   }

/*! \brief Computes sum of squares of elements in matrix

   \note This assumes that addition is defined for the type, in the accumulate
         operator (+=), as well as multiplication in the binary operator (*).
         Also, it is assumed that '0' is defined for the type.
*/
template <class T>
inline T matrix<T>::sumsq() const
   {
   assert(m_size > 0);
   T result = 0;
   for(int i=0; i<m_size.rows(); i++)
      for(int j=0; j<m_size.cols(); j++)
         result += m_data[i][j] * m_data[i][j];
   return result;
   }

/*! \brief Computes the variance of elements
*/
template <class T>
inline T matrix<T>::var() const
   {
   const T _mean = mean();
   const T _var = sumsq()/T(size()) - _mean*_mean;
   return (_var > 0) ? _var : 0;
   }

// static functions

/*!
   \brief Identity matrix
   \return The 'n'x'n' identity matrix
*/
template <class T>
inline matrix<T> matrix<T>::eye(int n)
   {
   assert(n > 0);
   matrix<T> r(n,n);
   r = 0;
   for(int i=0; i<n; i++)
      r(i,i) = 1;
   return r;
   }

}; // end namespace

/*!
   \brief Ordinary matrix power
   \return The result of A^n using ordinary matrix multiplication
   \note The use of 'i' and 'j' indices in this function follows the mathematical convention,
         rather than that used in the rest of this class.
*/
template <class T>
inline libbase::matrix<T> pow(const libbase::matrix<T>& A, int n)
   {
   using libbase::matrix;
   assert(A.size().rows() == A.size().cols());
   // power by zero return identity
   if(n == 0)
      return matrix<T>::eye(A.size().rows());
   // handle negative powers as powers of the inverse
   matrix<T> R;
   if(n > 0)
      R = A;
   else
      {
      R = A.inverse();
      n = -n;
      }
   // square for as long as possible
   for(; n>0 && n%2==0; n/=2)
      R *= R;
   // repeatedly multiply by A for whatever remains
   for(n--; n>0; n--)
      R *= A;
   return R;
   }


namespace libbase {

/*!
   \brief   Masked 2D Matrix.
   \author  Johann Briffa

   \section svn Version Control
   - $Revision$
   - $Date$
   - $Author$

   A masked matrix is a matrix with a binary element-mask. Arithmetic,
   statistical, user-defined operation, and copy/value init functions are
   defined for this class, allowing us to modify the masked parts of any
   given matrix with ease.

   It is intended that for the user, the use of masked matrices should be
   essentially transparent (in that they can mostly be used in place of
   normal matrices) and that the user should never create one explicitly,
   but merely through a normal matrix.
*/
template <class T>
class masked_matrix {
   friend class matrix<T>;
private:
   matrix<T>* m_data;
   matrix<bool> m_mask;
protected:
   masked_matrix(matrix<T>* data, const matrix<bool>& mask);
public:
   // matrix copy and value initialisation
   masked_matrix<T>& operator=(const T x);

   // convert to a vector
   operator vector<T>() const;

   // arithmetic operations - unary
   masked_matrix<T>& operator+=(const matrix<T>& x);
   masked_matrix<T>& operator-=(const matrix<T>& x);
private:
   //! Matrix multiplication is hidden because it's meaningless here
   masked_matrix<T>& operator*=(const matrix<T>& x) {};
   //! Matrix division is hidden because it's meaningless here
   masked_matrix<T>& operator/=(const matrix<T>& x) {};
public:
   masked_matrix<T>& multiplyby(const matrix<T>& x);
   masked_matrix<T>& divideby(const matrix<T>& x);
   masked_matrix<T>& operator+=(const T x);
   masked_matrix<T>& operator-=(const T x);
   masked_matrix<T>& operator*=(const T x);
   masked_matrix<T>& operator/=(const T x);

   // user-defined operations
   masked_matrix<T>& apply(T f(T));

   // information services
   int size() const; // number of masked-in elements

   // statistical operations
   T min() const;
   T max() const;
   T sum() const;
   T sumsq() const;
   T mean() const { return sum()/T(size()); };
   T var() const;
   T sigma() const { return sqrt(var()); };
};

// masked matrix constructor

template <class T>
inline masked_matrix<T>::masked_matrix(matrix<T>* data, const matrix<bool>& mask)
   {
   assert(data->m_size == mask.size());
   m_data = data;
   m_mask = mask;
   }

// matrix copy and value initialisation

template <class T>
inline masked_matrix<T>& masked_matrix<T>::operator=(const T x)
   {
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            m_data->m_data[i][j] = x;
   return *this;
   }

// convert to a vector

template <class T>
inline masked_matrix<T>::operator vector<T>() const
   {
   vector<T> v(size());
   int k=0;
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            v(k++) = m_data->m_data[i][j];
   return v;
   }

// arithmetic operations - unary

template <class T>
inline masked_matrix<T>& masked_matrix<T>::operator+=(const matrix<T>& x)
   {
   assert(x.m_size == m_data->m_size);
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            m_data->m_data[i][j] += x.m_data[i][j];
   return *this;
   }

template <class T>
inline masked_matrix<T>& masked_matrix<T>::operator-=(const matrix<T>& x)
   {
   assert(x.m_size == m_data->m_size);
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            m_data->m_data[i][j] -= x.m_data[i][j];
   return *this;
   }

/*!
   \brief Array multiplication (element-by-element) of matrices
   \param  x   Matrix to be multiplied to this one
   \return The updated (multiplied-into) matrix

   Masked elements (ie. where the mask is true) are multiplied by
   the corresponding element in 'x'. Unmasked elements are left
   untouched.

   \note For A.*B, the size of A must be the same as the size of B.
*/
template <class T>
inline masked_matrix<T>& masked_matrix<T>::multiplyby(const matrix<T>& x)
   {
   assert(x.m_size == m_data->m_size);
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            m_data->m_data[i][j] *= x.m_data[i][j];
   return *this;
   }

/*!
   \brief Array division (element-by-element) of matrices
   \param  x   Matrix to divide this one by
   \return The updated (divided-into) matrix

   Masked elements (ie. where the mask is true) are divided by
   the corresponding element in 'x'. Unmasked elements are left
   untouched.

   \note For A./B, the size of A must be the same as the size of B.
*/
template <class T>
inline masked_matrix<T>& masked_matrix<T>::divideby(const matrix<T>& x)
   {
   assert(x.m_size == m_data->m_size);
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            m_data->m_data[i][j] /= x.m_data[i][j];
   return *this;
   }

template <class T>
inline masked_matrix<T>& masked_matrix<T>::operator+=(const T x)
   {
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            m_data->m_data[i][j] += x;
   return *this;
   }

template <class T>
inline masked_matrix<T>& masked_matrix<T>::operator-=(const T x)
   {
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            m_data->m_data[i][j] -= x;
   return *this;
   }

template <class T>
inline masked_matrix<T>& masked_matrix<T>::operator*=(const T x)
   {
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            m_data->m_data[i][j] *= x;
   return *this;
   }

template <class T>
inline masked_matrix<T>& masked_matrix<T>::operator/=(const T x)
   {
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            m_data->m_data[i][j] /= x;
   return *this;
   }

// user-defined operations

template <class T>
inline masked_matrix<T>& masked_matrix<T>::apply(T f(T))
   {
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            m_data->m_data[i][j] = f(m_data->m_data[i][j]);
   return *this;
   }

// information services

template <class T>
inline int masked_matrix<T>::size() const
   {
   int result = 0;
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            result++;
   return result;
   }

// statistical operations

template <class T>
inline T masked_matrix<T>::min() const
   {
   assert(size() > 0);
   T result;
   bool initial = true;
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j) && (m_data->m_data[i][j] < result || initial))
            {
            result = m_data->m_data[i][j];
            initial = false;
            }
   return result;
   }

template <class T>
inline T masked_matrix<T>::max() const
   {
   assert(size() > 0);
   T result;
   bool initial = true;
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j) && (m_data->m_data[i][j] > result || initial))
            {
            result = m_data->m_data[i][j];
            initial = false;
            }
   return result;
   }

template <class T>
inline T masked_matrix<T>::sum() const
   {
   assert(m_data->m_size.rows() > 0);
   T result = 0;
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            result += m_data->m_data[i][j];
   return result;
   }

template <class T>
inline T masked_matrix<T>::sumsq() const
   {
   assert(m_data->m_size.rows() > 0);
   T result = 0;
   for(int i=0; i<m_data->m_size.rows(); i++)
      for(int j=0; j<m_data->m_size.cols(); j++)
         if(m_mask(i,j))
            result += m_data->m_data[i][j] * m_data->m_data[i][j];
   return result;
   }

template <class T>
inline T masked_matrix<T>::var() const
   {
   const T _mean = mean();
   const T _var = sumsq()/T(size()) - _mean*_mean;
   return (_var > 0) ? _var : 0;
   }

}; // end namespace

#endif
