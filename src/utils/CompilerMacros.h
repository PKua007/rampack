//
// Created by Piotr Kubala on 16/06/2023.
//

#ifndef RAMPACK_COMPILERMACROS_H
#define RAMPACK_COMPILERMACROS_H


#if defined(__GNUC__) && !defined(__clang__)
    #define RAMPACK_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#elif defined(__clang__)
    #define RAMPACK_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#else
    #define RAMPACK_UNKNOWN_COMPILER
#endif


#endif //RAMPACK_COMPILERMACROS_H
