#ifndef __fba2_cuda_h
#define __fba2_cuda_h

#include "config.h"
#include "vector.h"
#include "matrix.h"
#include "fsm.h"
#include "cuda-all.h"
#include "modem/dminner2-receiver.h"

#include <cmath>
#include <iostream>
#include <fstream>

namespace cuda {

// Determine debug level:
// 1 - Normal debug output only
// 2 - Show index computation for gamma vector
// NOTE: since this is a header, it may be included in other classes as well;
//       to avoid problems, the debug level is reset at the end of this file.
#ifndef NDEBUG
#  undef DEBUG
#  define DEBUG 1
#endif

/*!
 * \brief   Symbol-Level Forward-Backward Algorithm for CUDA.
 * \author  Johann Briffa
 *
 * \section svn Version Control
 * - $Revision$
 * - $Date$
 * - $Author$
 *
 * Implements the forward-backward algorithm for a HMM, as required for the
 * new decoder for Davey & McKay's inner codes, originally introduced in
 * "Watermark Codes: Reliable communication over Insertion/Deletion channels",
 * Trans. IT, 47(2), Feb 2001.
 *
 * \warning Do not use shorthand for class hierarchy, as these are not
 * interpreted properly by NVCC.
 */

template <class real, class sig, bool norm>
class fba2 {
public:
   /*! \name Type definitions */
   // Device-based types
   typedef cuda::vector<sig> dev_array1s_t;
   typedef cuda::vector<real> dev_array1r_t;
   typedef cuda::matrix<real> dev_array2r_t;
   typedef cuda::vector_reference<sig> dev_array1s_ref_t;
   typedef cuda::vector_reference<real> dev_array1r_ref_t;
   typedef cuda::matrix_reference<real> dev_array2r_ref_t;
   // Host-based types
   typedef libbase::vector<sig> array1s_t;
   typedef libbase::vector<double> array1d_t;
   typedef libbase::vector<real> array1r_t;
   typedef libbase::matrix<real> array2r_t;
   typedef libbase::vector<array1d_t> array1vd_t;
   typedef libbase::vector<array1r_t> array1vr_t;
   // @}
private:
   /*! \name User-defined parameters */
   int N; //!< The transmitted block size in symbols
   int n; //!< The number of bits encoding each q-ary symbol
   int q; //!< The number of symbols in the q-ary alphabet
   int I; //!< The maximum number of insertions considered before every transmission
   int xmax; //!< The maximum allowed overall drift is \f$ \pm x_{max} \f$
   int dxmax; //!< The maximum allowed drift within a q-ary symbol is \f$ \pm \delta_{max} \f$
   real th_inner; //!< Threshold factor for inner cycle
   real th_outer; //!< Threshold factor for outer cycle
   // @}
   /*! \name Internally-used objects */
   int dmin; //!< Offset for deltax index in gamma matrix
   int dmax; //!< Maximum value for deltax index in gamma matrix
   dev_array2r_ref_t alpha; //!< Forward recursion metric
   dev_array2r_ref_t beta; //!< Backward recursion metric
   dev_array1r_ref_t gamma; //!< Receiver metric
   mutable libcomm::dminner2_receiver<real> receiver; //!< Inner code receiver metric computation
   // @}
private:
   /*! \name Internal functions */
#ifdef __CUDACC__
   __device__ __host__
#endif
   int get_gamma_index(int d, int i, int x, int deltax) const
      {
      // gamma has indices (d,i,x,deltax) where:
      //    d in [0, q-1], i in [0, N-1], x in [-xmax, xmax], and
      //    deltax in [dmin, dmax] = [max(-n,-xmax), min(nI,xmax)]
      const int pitch3 = (dmax - dmin + 1);
      const int pitch2 = pitch3 * (2 * xmax + 1);
      const int pitch1 = pitch2 * N;
      const int off1 = d;
      const int off2 = i;
      const int off3 = x + xmax;
      const int off4 = deltax - dmin;
      const int ndx = off1 * pitch1 + off2 * pitch2 + off3 * pitch3 + off4;
#ifndef __CUDA_ARCH__
      // host code path only
#if DEBUG>=2
      std::cerr << "(" << d << "," << i << "," << x << "," << deltax << ":"
            << ndx << ")";
#endif
#endif
      return ndx;
      }
   // Device-only methods
#ifdef __CUDACC__
   __device__
   real get_gamma(int d, int i, int x, int deltax) const
      {
      return gamma(get_gamma_index(d, i, x, deltax));
      }
   __device__
   real& get_gamma(int d, int i, int x, int deltax)
      {
      return gamma(get_gamma_index(d, i, x, deltax));
      }
#endif
   // memory allocation
   void allocate(dev_array2r_t& alpha, dev_array2r_t& beta,
         dev_array1r_t& gamma);
   // helper methods
   void print_gamma(std::ostream& sout) const;
   // de-reference kernel calls
   void do_work_gamma(const dev_array1s_t& dev_r, const dev_array2r_t& dev_app);
   void do_work_gamma(const dev_array1s_t& dev_r);
   void do_work_alpha(int rho);
   void do_work_beta(int rho);
   void do_work_results(int rho, dev_array2r_t& dev_ptable) const;
   void copy_results(const dev_array2r_t& dev_ptable, array1vr_t& ptable);
   // @}
public:
   /*! \name Internal functions */
   // Device-only methods
#ifdef __CUDACC__
   __device__
   void work_gamma(const dev_array1s_ref_t& r, const dev_array2r_ref_t& app);
   __device__
   void work_gamma(const dev_array1s_ref_t& r);
   __device__
   static real get_threshold(const dev_array2r_ref_t& metric, int row, int cols, real factor);
   __device__
   static real parallel_sum(real array[]);
   __device__
   static real get_scale(const dev_array2r_ref_t& metric, int row, int cols);
   __device__
   static void normalize(dev_array2r_ref_t& metric, int row, int cols);
   __device__
   void normalize_alpha(int i)
      {
      normalize(alpha, i, 2 * xmax + 1);
      }
   __device__
   void normalize_beta(int i)
      {
      normalize(beta, i - 1, 2 * xmax + 1);
      }
   __device__
   void work_alpha(int rho, int i);
   __device__
   void work_beta(int rho, int i);
   __device__
   void work_results(int rho, dev_array2r_ref_t& ptable) const;
#endif
   // @}
public:
   /*! \name Constructors / Destructors */
   //! Default constructor
   fba2()
      {
      }
   // @}

   // main initialization routine
   void init(int N, int n, int q, int I, int xmax, int dxmax, double th_inner,
         double th_outer);
   // access metric computation
   libcomm::dminner2_receiver<real>& get_receiver() const
      {
      return receiver;
      }

   // decode functions
   void decode(const array1s_t& r, const array1vd_t& app, array1vr_t& ptable);
   void decode(const array1s_t& r, array1vr_t& ptable);

   // Description
   std::string description() const
      {
      return "Symbol-level Forward-Backward Algorithm [CUDA]";
      }
};

// Reset debug level, to avoid affecting other files
#ifndef NDEBUG
#  undef DEBUG
#  define DEBUG
#endif

} // end namespace

#endif
