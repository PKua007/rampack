//
// Created by Piotr Kubala on 27/08/2020.
//

#ifndef MBL_ED_OMPMACROS_H
#define MBL_ED_OMPMACROS_H

#ifdef _OPENMP
#include <omp.h>
    #define __OMP_STRINGIFY__(x) #x

    #define _OMP_PARALLEL_FOR   _Pragma("omp parallel for")
    #define _OMP_ATOMIC         _Pragma("omp atomic")
    #define _OMP_CRITICAL(x)    _Pragma(__OMP_STRINGIFY__(omp critical(x)))
    #define _OMP_MAXTHREADS     omp_get_max_threads()
    #define _OMP_THREAD_ID      omp_get_thread_num()
#else
#define _OMP_PARALLEL_FOR
#define _OMP_ATOMIC
#define _OMP_CRITICAL(x)
#define _OMP_MAXTHREADS     1
#define _OMP_THREAD_ID      0
#endif

#endif //MBL_ED_OMPMACROS_H