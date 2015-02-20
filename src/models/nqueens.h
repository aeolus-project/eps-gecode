/*---------------------------------------------------------------------------*/
/*                                                                           */
/* nqueens.h													    */
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

#ifndef __MODELS_NQUEENS_H__
#define __MODELS_NQUEENS_H__

#include "../flatzinc.h"
#include "../stl_util.h"

#include <list>

using namespace std;
using namespace Gecode;

class Queens : public MyFlatZincSpace {
public:
    /// Position of queens on boards
    //IntVarArray q;
    /// Propagation to use for model
    enum {
        PROP_BINARY = 1,  ///< Use only binary disequality constraints
        PROP_MIXED = 2,   ///< Use single distinct and binary disequality constraints
        PROP_DISTINCT = 0 ///< Use three distinct constraints
    };

    static MyFlatZincSpace* getInstance(MyFlatZincOptions& opt) {

        string name_instance("nqueens_" + stl_util::Convert2String(opt.nsize()));
        opt.name(name_instance.c_str());

        //opt.iterations(500);
        opt.propagation(Queens::PROP_DISTINCT);
        opt.icl(Gecode::ICL_VAL);
        /*
        optSize.propagation(Queens::PROP_BINARY, "binary",
                            "only binary disequality constraints");
        optSize.propagation(Queens::PROP_MIXED, "mixed",
                            "single distinct and binary disequality constraints");
        optSize.propagation(Queens::PROP_DISTINCT, "distinct",
                            "three distinct constraints");
        */
        return new Queens(opt);

    }

    /// The actual problem
    Queens(const MyFlatZincOptions& opt)
        : MyFlatZincSpace() {
        iv = IntVarArray(*this,opt.nsize(),0,opt.nsize()-1);
        _method = SAT;
        _optVar = -1;
        intVarCount = iv.size();
        boolVarCount = setVarCount = floatVarCount = 0;

        const IntVarArray& q = iv;
        const int n = iv.size();
        switch (opt.propagation()) {
        case PROP_BINARY:
            for (int i = 0; i<n; i++)
                for (int j = i+1; j<n; j++) {
                    rel(*this, q[i] != q[j]);
                    rel(*this, q[i]+i != q[j]+j);
                    rel(*this, q[i]-i != q[j]-j);
                }
            break;
        case PROP_MIXED:
            for (int i = 0; i<n; i++)
                for (int j = i+1; j<n; j++) {
                    rel(*this, q[i]+i != q[j]+j);
                    rel(*this, q[i]-i != q[j]-j);
                }
            distinct(*this, q, opt.icl());
            break;
        case PROP_DISTINCT:
        default:
            distinct(*this, IntArgs::create(n,0,1), q, opt.icl());
            distinct(*this, IntArgs::create(n,0,-1), q, opt.icl());
            distinct(*this, q, opt.icl());
            break;
        }

    }

    void createBranchers() {
        branch(*this, iv, Gecode::INT_VAR_SIZE_MIN(), Gecode::INT_VAL_MIN());
    }

    /// Constructor for cloning \a s
    /*
    Queens(bool share, Queens& s) : MyFlatZincSpace(share,s) {
        //q.update(*this, share, s.q);
    }
    */

    /// Perform copying during cloning
    /*
    virtual Space*
    copy(bool share) {
        return new Queens(share,*this);
    }
    */
    /// Print solution
    virtual void
    print(std::ostream& os) const {
        const IntVarArray& q = iv;
        os << "queens\t";
        for (int i = 0; i < q.size(); i++) {
            os << q[i] << ", ";
            if ((i+1) % 10 == 0)
                os << std::endl << "\t";
        }
        os << std::endl;
    }
};

#endif