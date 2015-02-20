/*---------------------------------------------------------------------------*/
/*                                                                           */
/* Lock.h - SpinLock and Mutex base class declaration.							 */
/*                                                                           */
/* Author : Mohamed REZGUI (m.rezgui06@gmail.com)                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/* Copyright (c) 2015 Mohamed REZGUI. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY MOHAMED REZGUI ``AS IS'' AND ANY
 * EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL MOHAMED REZGUI OR
 * ITS CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *----------------------------------------------------------------------------*/
#ifndef __LOCK_H__
#define __LOCK_H__

// ----- SpinLock -----

#if defined(unix)
#include <sched.h>
#endif

#if defined(__SVR4)
#include <thread.h>
#endif

#if defined(__sgi)
#include <mutex.h>
#endif

#if defined(_MSC_VER)

// Microsoft Visual Studio
#pragma inline_depth(255)
#define INLINE __forceinline
#define inline __forceinline
#define NO_INLINE __declspec(noinline)
#pragma warning(disable: 4530)
#define MALLOC_FUNCTION
#define RESTRICT

#elif defined(__GNUC__)

// GNU C

#define NO_INLINE       __attribute__ ((noinline))
#define INLINE          inline
#define MALLOC_FUNCTION __attribute__((malloc))
#define RESTRICT        __restrict__

#else

// All others

#define NO_INLINE
#define INLINE inline
#define MALLOC_FUNCTION
#define RESTRICT

#endif

#if defined(_MSC_VER)

#if !defined(NO_INLINE)
#pragma inline_depth(255)
#define NO_INLINE __declspec(noinline)
#define INLINE __forceinline
#define inline __forceinline
#endif

#else

#endif


#if defined(__SUNPRO_CC)
// x86-interchange.il, x86_64-interchange.il contributed by Markus Bernhardt.
extern "C" unsigned long MyInterlockedExchange (unsigned long * oldval,
        unsigned long newval);
#endif

#if defined(_WIN32) && !defined(_WIN64)

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif

// NOTE: Below is the new "pause" instruction, which is inocuous for
// previous architectures, but crucial for Intel chips with
// hyperthreading.  See
// http://www.usenix.org/events/wiess02/tech/full_papers/nakajima/nakajima.pdf
// for discussion
//pause instruction at spin-waits –
//The OS typically uses synchronization primitives, such as spin locks in multiprocessor systems.
//The pause is equivalent to “rep;nop” for all known Intel® architecture prior to Pentium® 4 or Intel® Xeon™ processors.
//The instruction in spin-waits can avoid severe penalty generated
//when a processor is spinning on a syn-chronization variable at full speed.

#define _MM_PAUSE  {__asm{_emit 0xf3};__asm {_emit 0x90}}
#include <windows.h>
#else
#define _MM_PAUSE
#endif

class SpinLock {
public:

    SpinLock (void)
        : mutex (UNLOCKED) {
    }

    ~SpinLock (void) {
    }

    inline void acquire (void) {
        // A yielding lock (with an initial spin).
        if (MyInterlockedExchange (const_cast<unsigned long *>(&mutex), LOCKED)
                != UNLOCKED) {
            contendedLock();
        }
    }


    inline void release (void) {
#if 1
#if defined(_WIN32) && !defined(_WIN64)
        __asm {}
#elif defined(__GNUC__)
        asm volatile ("" : : : "memory");
#endif
#endif
        mutex = UNLOCKED;
    }


#if !defined(__SUNPRO_CC)
    inline static volatile unsigned long MyInterlockedExchange (unsigned long *,unsigned long);
#endif

    private:

    NO_INLINE
    void contendedLock (void) {
        while (1) {
            if (MyInterlockedExchange (const_cast<unsigned long *>(&mutex), LOCKED)
                    == UNLOCKED) {
                return;
            }
            while (mutex == LOCKED) {
                _MM_PAUSE;
                yieldProcessor();
            }
        }
    }

    inline void yieldProcessor (void) {
#if defined(_WIN32)
        Sleep(0);
#else
#if defined(__SVR4)
        thr_yield();
#else
        sched_yield();
#endif
#endif
    }

    enum { UNLOCKED = 0, LOCKED = 1 };

    enum { MAX_SPIN_LIMIT = 1024 };

    union {
        //    double _dummy;
        volatile unsigned long mutex;
    };

};


// Atomically:
//   retval = *oldval;
//   *oldval = newval;
//   return retval;

#if !defined(__SUNPRO_CC)
inline volatile unsigned long
SpinLock::MyInterlockedExchange (unsigned long * oldval,
                                 unsigned long newval) {
#if defined(_WIN32) && defined(_MSC_VER)
    return InterlockedExchange ((volatile LONG *) oldval, newval);

#elif defined(__sparc)
    asm volatile ("swap [%1],%0"
                  :"=r" (newval)
                  :"r" (oldval), "0" (newval)
                  : "memory");

#elif defined(__i386__)
    asm volatile ("lock; xchgl %0, %1"
                  : "=r" (newval)
                  : "m" (*oldval), "0" (newval)
                  : "memory");

#elif defined(__sgi)
    newval = test_and_set (oldval, newval);

#elif defined(__x86_64__)
    // Contributed by Kurt Roeckx.
    asm volatile ("lock; xchgq %0, %1"
                  : "=r" (newval)
                  : "m" (*oldval), "0" (newval)
                  : "memory");

#elif defined(__APPLE__) || defined(__ppc) || defined(__powerpc__) || defined(PPC)
    // PPC assembly contributed by Maged Michael.
    int ret;
    asm volatile (
        "La..%=0:    lwarx %0,0,%1 ;"
        "      cmpw  %0,%2;"
        "      beq La..%=1;"
        "      stwcx. %2,0,%1;"
        "      bne- La..%=0;"
        "La..%=1:    isync;"
        : "=&r"(ret)
        : "r"(oldval), "r"(newval)
        : "cr0", "memory");
    return ret;

#elif defined(__arm__)
    // Contributed by Bo Granlund.
    long result;
    asm volatile (
        "\n\t"
        "swp     %0,%2,[%1] \n\t"
        ""
        : "=&r"(result)
        : "r"(oldval), "r"(newval)
        : "memory");
    return (result);
#else
#error "No spin lock implementation is available for this platform."
#endif
    return newval;
}

#endif

class MyLockFile {
    public:
    MyLockFile (std::string const & lock_file);
    ~MyLockFile ();

    void lock () const;
    void unlock () const;
    int read(void *buffer,  int nbyte);
    int write(void *buffer,  int nbyte);

    private:
    MyLockFile (MyLockFile&);
    MyLockFile &operator= (const MyLockFile&);

    void open (int) const;
    void close () const;

    struct Impl;

    std::string lock_file_name;
    Impl * data;
};

#endif