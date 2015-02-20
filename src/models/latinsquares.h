/*---------------------------------------------------------------------------*/
/*                                                                           */
/* latinsquares.h													    */
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

#ifndef __MODELS_LATINSQUARES_H__
#define __MODELS_LATINSQUARES_H__

#include "../flatzinc.h"
#include "../stl_util.h"

#include <list>

using namespace std;
using namespace Gecode;

class LatinSquares : public MyFlatZincSpace {
protected:

    int n;          // size of the grid
    //IntVarArray x;  // the grid

public:

    // Symmetry options
    enum {
        SYMMETRY_NONE,
        SYMMETRY_MIN    // use symmetry breaking
    };

    static MyFlatZincSpace* getInstance(MyFlatZincOptions& opt) {

        string name_instance("latinsquares_" + stl_util::Convert2String(opt.nsize()));
        opt.name(name_instance.c_str());

        opt.symmetry(LatinSquares::SYMMETRY_MIN);
        /*
        opt.symmetry(LatinSquares::SYMMETRY_NONE, "none", "do not use symmetry");
        opt.symmetry(LatinSquares::SYMMETRY_MIN, "min", "minimum element first");
        */

        return new LatinSquares(opt);
    }

    LatinSquares(const MyFlatZincOptions& opt)
        : MyFlatZincSpace(),
          n(opt.nsize()) {
        this->needAuxVars = false;
        iv = IntVarArray(*this, n*n, 0, n);
        _method = SAT;
        _optVar = -1;
        intVarCount = iv.size();
        boolVarCount = setVarCount = floatVarCount = 0;

        // Matrix wrapper for the x grid
        Matrix<IntVarArray> m(iv, n, n);

        //latin_square(*this, m, opt.icl());
        // we assume that the matrix is a square

        //int w = m.width();
        for(int i = 0; i < n; i++) {
            distinct(*this, m.row(i), Gecode::ICL_DOM);
            distinct(*this, m.col(i), Gecode::ICL_DOM);
        }


        //for latinsquares
        // Symmetry breaking. 0 is upper left column
        if (opt.symmetry() == SYMMETRY_MIN) {
            rel(*this, iv[0] == 1, opt.icl());
        }



    }

    void createBranchers() {
        // branching
        branch(*this, iv, INT_VAR_SIZE_MIN(), INT_VAL_MIN());
        //branch(*this, iv, INT_VAR_NONE(), INT_VAL_MIN());
        //branch(*this, iv, INT_VAR_SIZE_MIN(), INT_VAL_RANGE_MAX());
    }

    // Print the solution
    virtual void
    print(std::ostream& os) const {
        for(int i = 0; i < n; i++) {
            for(int j = 0; j < n; j++) {
                os << iv[i*n+j].val() << " ";
            }
            os << endl;
        }
        os << endl;
    }

    // Constructor for cloning s
    LatinSquares(bool share, LatinSquares& s) : MyFlatZincSpace(share, s), n(s.n) {}

    // Copy during cloning
    virtual Space*
    copy(bool share) {
        return new LatinSquares(share,*this);
    }
};

#endif