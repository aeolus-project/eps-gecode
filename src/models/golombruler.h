/*---------------------------------------------------------------------------*/
/*                                                                           */
/* golombruler.h													    */
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

#ifndef __MODELS_GOLOMBRULER_H__
#define __MODELS_GOLOMBRULER_H__

#include "../flatzinc.h"
#include "../stl_util.h"

#include <list>

using namespace std;
using namespace Gecode;

// kG[n] = G(n).
static const int kG[] = {
    -1, 0, 1, 3, 6, 11, 17, 25, 34, 44, 55, 72, 85,
    106, 127, 151, 177, 199, 216, 246
};

class GolombRuler : public MyFlatZincSpace {
protected:
    /// Array for ruler marks
    //IntVarArray m;
public:

    static MyFlatZincSpace* getInstance(MyFlatZincOptions& opt) {

        string name_instance("golombruler_" + stl_util::Convert2String(opt.nsize()));
        opt.name(name_instance.c_str());

        opt.icl(ICL_BND);

        return new GolombRuler(opt);
    }

    /// Actual model
    GolombRuler(const MyFlatZincOptions& opt)
        : MyFlatZincSpace() {

        const int max = kG[opt.nsize()];
        iv = IntVarArray(*this,opt.nsize(), 0, max);

        //iv = IntVarArray(*this,opt.size(), 0,
        //                 (opt.size() < 31) ? (1 << (opt.size()-1))-1 : Int::Limits::max);

        _method = MIN;
        _optVar = iv.size()-1;
        intVarCount = iv.size();
        boolVarCount = setVarCount = floatVarCount = 0;

        // Assume first mark to be zero
        rel(*this, iv[0], IRT_EQ, 0);

        // Order marks
        rel(*this, iv, IRT_LE);


        // Number of marks and differences
        const int n = iv.size();
        const int n_d = (n*n-n)/2;

        // Array of differences
        IntVarArgs d(n_d);

        // Setup difference constraints
        for (int k=0, i=0; i<n-1; i++)
            for (int j=i+1; j<n; j++, k++)
                // d[k] is m[j]-m[i] and must be at least sum of first j-i integers
                rel(*this, d[k] = expr(*this, iv[j]-iv[i]),
                    IRT_GQ, (j-i)*(j-i+1)/2);

        distinct(*this, d, opt.icl());

        // Symmetry breaking
        if (n > 2) {
            rel(*this, d[0], IRT_LE, d[n_d-1]);
        }

    }

    void createBranchers() {
        branch(*this, iv, INT_VAR_NONE(), INT_VAL_MIN());
    }

    /// Return cost
    /*
    virtual IntVar cost(void) const {
      return m[m.size()-1];
    }
    */
    /// Print solution
    virtual void
    print(std::ostream& os) const {
        os << "\tm[" << iv.size() << "] = " << iv << std::endl;
    }
    /*
    /// Constructor for cloning \a s
    GolombRuler(bool share, GolombRuler& s)
      : MyFlatZincSpace(share,s) {
    }
    /// Copy during cloning
    virtual Space*
    copy(bool share) {
      return new GolombRuler(share,*this);
    }
    */
};

#endif