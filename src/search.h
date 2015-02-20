/*---------------------------------------------------------------------------*/
/*                                                                           */
/* search.h													    */
/*                                                                           */
/* Author : Mohamed REZGUI (m.rezgui06@gmail.com)                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/* Copyright (c) 2014 Mohamed REZGUI. All rights reserved.
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

#ifndef __MY_SEARCH_H__
#define __MY_SEARCH_H__

#include <gecode/search.hh>
#include <gecode/search/sequential/dfs.hh>

#include "flatzinc.h"

class MySearchOptions : public Gecode::Search::Options {
public:
    unsigned int nb_problems;
    unsigned int mode_decomposition;
    unsigned int mode_search;
    std::string  obj_file;   ///< objective file path
    unsigned int first_level;

    MySearchOptions() : Gecode::Search::Options(), nb_problems(50), mode_decomposition(0), mode_search(0), obj_file(), first_level(0) {
    }
    MySearchOptions(const Gecode::Search::Options& opt) : Gecode::Search::Options(opt), nb_problems(50), mode_decomposition(0), mode_search(0), obj_file(), first_level(0) {
    }

    MySearchOptions(const MySearchOptions& opt) : Gecode::Search::Options(opt),
        nb_problems(opt.nb_problems), mode_decomposition(opt.mode_decomposition), mode_search(opt.mode_search), obj_file(opt.obj_file), first_level(opt.first_level) {
    }

};

/// Depth-first search engine implementation with limited depth
class BoundedDFS : public Gecode::Search::Worker {
private:
    /// Search options
    Gecode::Search::Options opt;
    /// Current path ins search tree
    Gecode::Search::Sequential::Path path;
    /// Distance until next clone
    unsigned int d;
public:
    /// Current space being explored
    Gecode::Space* cur;

    /// Initialize for space \a s (of size \a sz) with options \a o
    BoundedDFS(Gecode::Space* s, const Gecode::Search::Options& o);
    /// %Search for next solution
    Gecode::Space* next(void);
    /// Return statistics
    Gecode::Search::Statistics statistics(void) const;
    /// Reset engine to restart at space \a s
    void reset(Gecode::Space* s);
    /// Return no-goods
    Gecode::NoGoods& nogoods(void);
    /// Destructor
    ~BoundedDFS(void);
};


/**
 * \brief Depth-first search engine
 *
 * This class supports depth-first search for subclasses \a T of
 * Space.
 * \ingroup TaskModelSearch
 */
template<class T>
class EPS_DFS : public Gecode::EngineBase {
public:
    /// Initialize search engine for space \a s with options \a o
    EPS_DFS(T* s, const MySearchOptions& o=Gecode::Search::Options::def);
    /// Return next solution (NULL, if none exists or search has been stopped)
    T* next(void);
    /// Return statistics
    Gecode::Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    bool stopped(void) const;
    /// Return no-goods
    Gecode::NoGoods& nogoods(void);
};

/// Invoke depth-first search engine for subclass \a T of space \a s with options \a o
template<class T>
T* eps_dfs(T* s, const MySearchOptions& o=Gecode::Search::Options::def);

#include "eps_dfs.hpp"

/**
 * \brief Depth-first branch-and-bound search engine
 *
 * Additionally, \a s must implement a member function
 * \code virtual void constrain(const T& t) \endcode
 * Whenever exploration requires to add a constraint
 * to the space \a c currently being explored, the engine
 * executes \c c.constrain(t) where \a t is the so-far
 * best solution.
 * \ingroup TaskModelSearch
 */
template<class T>
class EPS_BAB : public Gecode::EngineBase {
public:
    /// Initialize engine for space \a s and options \a o
    EPS_BAB(T* s, const MySearchOptions& o=Gecode::Search::Options::def);
    /// Return next better solution (NULL, if none exists or search has been stopped)
    T* next(void);
    /// Return statistics
    Gecode::Search::Statistics statistics(void) const;
    /// Check whether engine has been stopped
    bool stopped(void) const;
    /// Return no-goods
    Gecode::NoGoods& nogoods(void);
};

/**
 * \brief Perform depth-first branch-and-bound search for subclass \a T of space \a s and options \a o
 *
 * Additionally, \a s must implement a member function
 * \code virtual void constrain(const T& t) \endcode
 * Whenever exploration requires to add a constraint
 * to the space \a c currently being explored, the engine
 * executes \c c.constrain(t) where \a t is the so-far
 * best solution.
 *
 * \ingroup TaskModelSearch
 */
template<class T>
T* eps_bab(T* s, const MySearchOptions& o=Gecode::Search::Options::def);

#include "eps_bab.hpp"

#endif
