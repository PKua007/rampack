//
// Created by Piotr Kubala on 27/08/2020.
//

#ifndef MBL_ED_OMPMACROS_H
#define MBL_ED_OMPMACROS_H

#ifdef _OPENMP
#include <omp.h>
    #define OMP_MAXTHREADS              omp_get_max_threads()
    #define OMP_SET_NUM_THREADS(num)    omp_set_num_threads(num)
    #define OMP_THREAD_ID               omp_get_thread_num()
#else
    #define OMP_MAXTHREADS              1
    #define OMP_SET_NUM_THREADS(num)    static_cast<void>(num)
    #define OMP_THREAD_ID               0
#endif

#endif //MBL_ED_OMPMACROS_H