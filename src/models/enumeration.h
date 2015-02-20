/*---------------------------------------------------------------------------*/
/*                                                                           */
/* enumeration.h													    */
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

#ifndef __MODELS_ENUMERATION_H__
#define __MODELS_ENUMERATION_H__

#include "../flatzinc.h"
#include "../stl_util.h"

#include <list>

using namespace std;
using namespace Gecode;

class Enumeration : public MyFlatZincSpace {
protected:

    int n;
    int m;

public:

    static MyFlatZincSpace* getInstance(MyFlatZincOptions& opt) {

        string name_instance("enumeration_" + stl_util::Convert2String(opt.nsize()) + "_" + stl_util::Convert2String(opt.msize()));
        opt.name(name_instance.c_str());

        opt.icl(ICL_DOM);

        return new Enumeration(opt);
    }

    Enumeration(const MyFlatZincOptions& opt)
        : MyFlatZincSpace(),
          n(opt.nsize()),
          m(opt.msize()) {

        iv = IntVarArray(*this, n);
        int* values = new int[m];
		const int* ptrV = values;
        for(int k = 0; k < n; k++) {
            for(int i = 0; i < m; i++) {
                values[i] = 2*i;//(k+i)*(k+i);
            }

            IntSet values(ptrV, m);

			iv[k] = IntVar(*this, values);
        }
		delete values;
        //iv.update(*this, true, args);
        _method = SAT;
        _optVar = -1;
        intVarCount = iv.size();
        boolVarCount = setVarCount = floatVarCount = 0;


    }

    void createBranchers() {
        // branching
        branch(*this, iv, Gecode::INT_VAR_NONE(), INT_VAL_MIN());
    }

    // Print solution
    virtual void
    print(std::ostream& os) const {
        os << "x    : " << iv << endl;
        os << "n    : " << n << endl;
        os << "m    : " << m << endl;
        os << endl;
    }


    // Constructor for cloning s
    Enumeration(bool share, Enumeration& s) : MyFlatZincSpace(share,s), n(s.n) {
    }

    // Copy during cloning
    virtual Space*
    copy(bool share) {
        return new Enumeration(share,*this);
    }
};

#endif