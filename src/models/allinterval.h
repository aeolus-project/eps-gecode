/*---------------------------------------------------------------------------*/
/*                                                                           */
/* allinterval.h													    */
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

#ifndef __MODELS_ALLINTERVAL_H__
#define __MODELS_ALLINTERVAL_H__

#include "../flatzinc.h"
#include "../stl_util.h"

#include <list>

using namespace std;
using namespace Gecode;

class AllInterval : public MyFlatZincSpace {
protected:

    int n;

    //IntVarArray iv;
    IntVarArray diffs;

public:

    static MyFlatZincSpace* getInstance(MyFlatZincOptions& opt) {

        string name_instance("allinterval_" + stl_util::Convert2String(opt.nsize()));
        opt.name(name_instance.c_str());

        opt.icl(ICL_DOM);

        return new AllInterval(opt);
    }

    AllInterval(const MyFlatZincOptions& opt)
        : MyFlatZincSpace(),
          n(opt.nsize()),
          diffs(*this, n-1, 1, n-1)

    {
        iv = IntVarArray(*this, n, 0, n-1);
        _method = SAT;
        _optVar = -1;
        intVarCount = iv.size();
        boolVarCount = setVarCount = floatVarCount = 0;

        Gecode::TupleSet set;
        int i,j;
        for(i=0; i<n-1; i++) {
            for(j=i+1; j<n; j++) {
                IntArgs tuple(3);
                tuple[0] = i;
                tuple[1] = j;
                tuple[2] = abs(j-i);
                set.add(tuple);

                tuple[0] = j;
                tuple[1] = i;
                tuple[2] = abs(j-i);
                set.add(tuple);
            }
        }
        set.finalize();
        for (i=0; i<n-1; i++) {
            IntVarArgs vars(3);
            vars[0] = iv[i];
            vars[1] = iv[i+1];
            vars[2] = diffs[i];
            Gecode::extensional(*this, vars, set);
        }

        //cout << "Size: " << n << endl;

        distinct(*this, iv, opt.icl());
        distinct(*this, diffs, opt.icl());
        /*
        for(int k = 0; k < n-1; k++) {
            rel(*this, diffs[k] == abs(iv[k+1] - iv[k]), opt.icl());
        }
        // symmetry breaking
        rel(*this, iv[0] < iv[n-1], opt.icl());
        rel(*this, diffs[0] < diffs[1], opt.icl());
        */

    }

    void createBranchers() {
        // branching
        branch(*this, iv, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
    }

    // Print solution
    virtual void
    print(std::ostream& os) const {
        os << "x    : " << iv << endl;
        os << "diffs: " << diffs << endl;
        os << endl;
    }


    // Constructor for cloning s
    AllInterval(bool share, AllInterval& s) : MyFlatZincSpace(share,s), n(s.n) {
        diffs.update(*this, share, s.diffs);
    }

    // Copy during cloning
    virtual Space*
    copy(bool share) {
        return new AllInterval(share,*this);
    }
};

#endif