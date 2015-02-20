/*---------------------------------------------------------------------------*/
/*                                                                           */
/* eps_dfs.hpp													    */
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

#ifndef __EPS_DFS_HPP__
#define __EPS_DFS_HPP__

// Create depth-first engine
Gecode::Search::Engine* eps_dfs_to_engine(Gecode::Space* s, const MySearchOptions& o);

template<class T>
forceinline
EPS_DFS<T>::EPS_DFS(T* s, const MySearchOptions& o)
    : EngineBase(eps_dfs_to_engine(s,o)) {}

template<class T>
forceinline T*
EPS_DFS<T>::next(void) {
    return dynamic_cast<T*>(e->next());
}

template<class T>
forceinline Gecode::Search::Statistics
EPS_DFS<T>::statistics(void) const {
    return e->statistics();
}

template<class T>
forceinline bool
EPS_DFS<T>::stopped(void) const {
    return e->stopped();
}

template<class T>
forceinline Gecode::NoGoods&
EPS_DFS<T>::nogoods(void) {
    return e->nogoods();
}

template<class T>
forceinline T*
eps_dfs(T* s, const MySearchOptions& o) {
    EPS_DFS<T> d(s,o);
    return d.next();
}

#endif
