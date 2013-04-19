/*!
 * \file
 * 
 * Copyright (c) 2010 Johann A. Briffa
 * 
 * This file is part of SimCommSys.
 *
 * SimCommSys is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SimCommSys is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SimCommSys.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * \section svn Version Control
 * - $Id$
 */

#ifndef __fba_generic_h
#define __fba_generic_h

#include "config.h"
#include "vector.h"
#include "multi_array.h"
#include "channel/qids.h"

#include <cmath>
#include <iostream>
#include <fstream>

namespace libcomm {

/*!
 * \brief   Generic Forward-Backward Algorithm (for Marker codes).
 * \author  Johann Briffa
 *
 * \section svn Version Control
 * - $Revision$
 * - $Date$
 * - $Author$
 *
 * Implements a generic Forward-Backward Algorithm for decoding bits received
 * from a random insertion-deletion channel. This is meant primarily for
 * marker codes as described in
 * Ratzer, "Error-correction on non-standard communication channels",
 * PhD dissertation, University of Cambridge, 2003.
 * This class implements decoding using a trellis-based implementation of the
 * forward-backward algorithm, rather than a lattice-based one.
 *
 * \tparam sig Channel symbol type
 * \tparam real Floating-point type for internal computation
 * \tparam real2 Floating-point type for internal computation
 */

template <class sig, class real, class real2>
class fba_generic {
private:
   // Shorthand for class hierarchy
   typedef fba_generic<sig, real, real2> This;
public:
   /*! \name Type definitions */
   typedef libbase::vector<sig> array1s_t;
   typedef libbase::vector<double> array1d_t;
   typedef libbase::vector<real> array1r_t;
   typedef libbase::vector<array1d_t> array1vd_t;
   typedef libbase::vector<array1r_t> array1vr_t;
   typedef boost::assignable_multi_array<real, 2> array2r_t;
   // @}
private:
   /*! \name User-defined parameters */
   int N; //!< The (transmitted) block size in bits
   int I; //!< The maximum number of insertions per time-step
   int xmax; //!< The maximum allowed drift overall
   bool norm; //!< Flag to indicate if metrics should be normalized between time-steps
   typename qids<sig, real2>::metric_computer computer; //!< Channel object for computing receiver metric
   // @}
   /*! \name Internally-used objects */
   bool initialised; //!< Flag to indicate when memory is allocated
   array2r_t alpha; //!< Forward recursion metric
   array2r_t beta; //!< Backward recursion metric
   // @}
private:
   /*! \name Internal functions */
   // memory allocation
   void allocate();
   void free();
   // common small tasks
   static real get_scale(const array2r_t& metric, int row, int col_min,
         int col_max);
   static void normalize(array2r_t& metric, int row, int col_min, int col_max);
   void normalize_alpha(int i)
      {
      normalize(alpha, i, -xmax, xmax);
      }
   void normalize_beta(int i)
      {
      normalize(beta, i, -xmax, xmax);
      }
   // decode functions
   void work_alpha(const array1s_t& r, const array1vd_t& app,
         const array1d_t& sof_prior);
   void work_beta(const array1s_t& r, const array1vd_t& app,
         const array1d_t& eof_prior);
   void work_message_app(const array1s_t& r, const array1vd_t& app,
         array1vr_t& ptable) const;
   void work_state_app(array1r_t& ptable, const int i) const;
   void work_results(const array1s_t& r, const array1vd_t& app,
         array1vr_t& ptable, array1r_t& sof_post, array1r_t& eof_post) const
      {
      assert(initialised);
      // compute APPs of message
      work_message_app(r, app, ptable);
      // compute APPs of sof/eof state values
      work_state_app(sof_post, 0);
      work_state_app(eof_post, N);
      }
   // @}
public:
   /*! \name Constructors / Destructors */
   //! Default constructor
   fba_generic() :
         initialised(false)
      {
      }
   // @}

   /*! \brief Set up code size and channel receiver
    * Only needs to be done before the first frame.
    */
   void init(int N, int I, int xmax, bool norm,
         const libcomm::qids<sig, real2>& chan)
      {
      // if any parameters that effect memory have changed, release memory
      if (initialised && (N != This::N || xmax != This::xmax))
         free();
      // code parameters
      assert(N > 0);
      This::N = N;
      // decoder parameters
      assert(I > 0);
      assert(xmax > 0);
      This::I = I;
      This::xmax = xmax;
      // decoding mode parameters
      This::norm = norm;
      // channel receiver
      computer = chan.get_computer();
      }

   /*! \name Parameter getters */
   int get_I() const
      {
      return I;
      }
   int get_xmax() const
      {
      return xmax;
      }
   // @}

   // decode functions
   void decode(libcomm::instrumented& collector, const array1s_t& r,
         const array1d_t& sof_prior, const array1d_t& eof_prior,
         const array1vd_t& app, array1vr_t& ptable, array1r_t& sof_post,
         array1r_t& eof_post, const int offset);
   void get_drift_pdf(array1r_t& pdf, const int i) const
      {
      work_state_app(pdf, i);
      }

   // Description
   std::string description() const
      {
      return "Generic Forward-Backward Algorithm";
      }
};

} // end namespace

#endif
