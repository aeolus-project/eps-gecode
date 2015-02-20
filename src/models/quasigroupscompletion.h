/*---------------------------------------------------------------------------*/
/*                                                                           */
/* quasigroupscompletion.h													    */
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

#ifndef __MODELS_QUASIGROUPSCOMPLETION_H__
#define __MODELS_QUASIGROUPSCOMPLETION_H__

#include "../flatzinc.h"
#include "../stl_util.h"

#include <list>

using namespace std;
using namespace Gecode;

class QuasiGroupsCompletion : public MyFlatZincSpace {
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

        string name_instance("quasigroupscompletion_" + stl_util::Convert2String(opt.nsize()));
        opt.name(name_instance.c_str());


        return new QuasiGroupsCompletion(opt);
    }

    QuasiGroupsCompletion(const MyFlatZincOptions& opt)
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
        /*
        int _m[] = {
            0, 0, 1, 5, 2, 6, 7, 8, 0, 0,
            0, 1, 5, 2, 0, 0, 6, 7, 8, 0,
            1, 5, 2, 0, 0, 0, 0, 6, 7, 8,
            5, 2, 0, 0, 0, 0, 0, 0, 6, 7,
            2, 0, 0, 0, 0, 0, 0, 0, 0, 6,
            4,10, 0, 0, 0, 0, 0, 0, 3, 9,
            0, 4,10, 0, 0, 0, 0, 3, 9, 0,
            0, 0, 4,10, 0, 0, 3, 9, 0, 0,
            0, 0, 0, 4,10, 3, 9, 0, 0, 0,
            0, 0, 0, 0, 4, 9, 0, 0, 0, 0
        };
        */
        int _m[] = {
            -1,   1,   2,  -1,   4,   5,   6,   7,   8,  -1,  10,  -1,  -1,  -1,  -1,  15,  16,  17,  -1,  19,  20,  21,  -1,  23,  -1,  -1,  -1,  27,  -1,  29,
            1,   2,   3,  -1,   5,   6,  -1,   8,   9,  -1,  11,  -1,  13,  14,  -1,  16,  17,  -1,  19,  20,  21,  -1,  23,  -1,  25,  26,  -1,  28,  29,  -1,
            -1,   3,   4,   5,   6,   7,   8,   9,  -1,  -1,  -1,  13,  14,  15,  -1,  17,  18,  19,  20,  21,  22,  23,  -1,  25,  26,  27,  -1,  29,   0,   1,
            -1,   4,  -1,   6,   7,   8,   9,  10,  11,  -1,  13,  14,  15,  -1,  17,  18,  19,  20,  21,  22,  -1,  24,  25,  26,  27,  28,  29,   0,   1,  -1,
            -1,  -1,   6,   7,  -1,  -1,  -1,  11,  -1,  -1,  14,  15,  16,  17,  18,  -1,  20,  21,  -1,  23,  24,  -1,  26,  27,  28,  29,  -1,  -1,   2,   3,
            5,   6,  -1,  -1,  -1,  10,  11,  12,  13,  14,  15,  16,  -1,  18,  19,  20,  21,  -1,  23,  -1,  -1,  -1,  -1,  28,  -1,   0,   1,  -1,  -1,   4,
            -1,  -1,   8,   9,  10,  11,  12,  13,  14,  -1,  16,  17,  18,  -1,  20,  -1,  -1,  -1,  24,  25,  26,  -1,  -1,  -1,   0,   1,   2,   3,  -1,  -1,
            -1,   8,  -1,  10,  11,  12,  13,  14,  -1,  16,  17,  -1,  19,  20,  -1,  -1,  23,  24,  25,  26,  27,  28,  -1,   0,  -1,   2,   3,  -1,  -1,  -1,
            8,   9,  -1,  11,  12,  13,  14,  15,  -1,  17,  18,  19,  20,  21,  22,  23,  24,  -1,  -1,  27,  28,  29,  -1,  -1,   2,   3,   4,   5,   6,  -1,
            -1,  10,  -1,  -1,  -1,  -1,  -1,  16,  17,  18,  19,  20,  21,  -1,  -1,  24,  25,  26,  -1,  -1,  29,  -1,   1,   2,   3,   4,  -1,  -1,   7,   8,
            10,  11,  12,  13,  14,  -1,  16,  -1,  18,  19,  20,  21,  -1,  -1,  24,  25,  -1,  27,  28,  29,   0,   1,   2,   3,  -1,   5,   6,   7,  -1,   9,
            11,  12,  -1,  14,  15,  -1,  17,  -1,  19,  -1,  21,  22,  -1,  24,  25,  26,  -1,  -1,  -1,   0,   1,   2,   3,  -1,   5,   6,  -1,   8,  -1,  10,
            -1,  -1,  14,  15,  -1,  -1,  -1,  19,  20,  21,  22,  -1,  24,  -1,  26,  27,  28,  -1,  -1,   1,   2,   3,   4,   5,   6,   7,  -1,   9,  10,  11,
            -1,  -1,  15,  16,  -1,  18,  19,  -1,  21,  -1,  -1,  24,  -1,  26,  27,  28,  -1,   0,   1,   2,   3,  -1,  -1,   6,   7,   8,   9,  10,  -1,  12,
            -1,  15,  16,  -1,  -1,  19,  -1,  21,  22,  23,  -1,  25,  26,  -1,  -1,  29,  -1,   1,  -1,   3,  -1,   5,   6,  -1,  -1,   9,  10,  11,  12,  13,
            -1,  16,  17,  -1,  -1,  20,  21,  22,  23,  24,  25,  26,  -1,  28,  29,   0,   1,  -1,  -1,  -1,   5,  -1,  -1,   8,   9,  -1,  -1,  12,  -1,  14,
            -1,  -1,  18,  19,  20,  21,  22,  -1,  24,  25,  26,  -1,  28,  29,  -1,  -1,   2,   3,   4,   5,   6,   7,  -1,  -1,  10,  11,  -1,  -1,  14,  15,
            -1,  18,  19,  -1,  -1,  22,  -1,  -1,  25,  26,  27,  28,  29,   0,   1,  -1,   3,  -1,  -1,   6,   7,   8,   9,  10,  -1,  12,  13,  -1,  15,  16,
            18,  -1,  -1,  -1,  -1,  23,  24,  25,  -1,  -1,  -1,  -1,   0,  -1,   2,  -1,   4,  -1,  -1,   7,   8,  -1,  10,  11,  12,  -1,  14,  15,  -1,  -1,
            -1,  -1,  21,  22,  -1,  -1,  25,  -1,  27,  28,  29,   0,  -1,  -1,   3,   4,   5,  -1,   7,   8,   9,  -1,  11,  -1,  13,  -1,  15,  -1,  -1,  18,
            20,  21,  22,  -1,  24,  25,  26,  -1,  28,  29,  -1,   1,   2,   3,   4,   5,  -1,   7,   8,  -1,  -1,  -1,  12,  -1,  -1,  15,  16,  17,  18,  -1,
            21,  22,  23,  24,  25,  26,  27,  28,  -1,   0,   1,   2,  -1,   4,   5,  -1,  -1,   8,  -1,  10,  11,  12,  -1,  14,  -1,  16,  17,  18,  -1,  20,
            22,  23,  24,  -1,  -1,  27,  -1,  -1,  -1,  -1,  -1,   3,   4,  -1,   6,  -1,  -1,  -1,  -1,  11,  12,  -1,  -1,  -1,  -1,  17,  18,  -1,  -1,  -1,
            23,  -1,  -1,  26,  -1,  -1,  29,   0,   1,  -1,  -1,   4,  -1,  -1,   7,  -1,  -1,  10,  -1,  12,  13,  14,  15,  16,  -1,  18,  19,  20,  -1,  -1,
            24,  -1,  26,  27,  28,  29,   0,   1,   2,   3,   4,   5,   6,   7,  -1,  -1,  -1,  11,  -1,  13,  -1,  15,  16,  17,  18,  19,  -1,  21,  22,  23,
            25,  26,  27,  28,  -1,   0,  -1,   2,   3,  -1,   5,  -1,   7,   8,   9,  10,  11,  -1,  13,  14,  -1,  16,  -1,  -1,  19,  20,  -1,  -1,  23,  24,
            26,  -1,  -1,  -1,  -1,   1,   2,   3,  -1,   5,   6,   7,   8,   9,  -1,  -1,  12,  13,  14,  15,  16,  -1,  18,  -1,  20,  21,  -1,  23,  24,  -1,
            27,  28,  29,  -1,   1,   2,   3,  -1,  -1,  -1,  -1,  -1,   9,  10,  11,  12,  13,  14,  -1,  -1,  17,  18,  19,  20,  -1,  22,  23,  24,  25,  26,
            -1,  -1,   0,  -1,  -1,   3,  -1,  -1,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,  16,  17,  18,  19,  20,  -1,  22,  23,  24,  25,  26,  -1,
            29,   0,  -1,   2,   3,   4,   5,   6,  -1,   8,  -1,  10,  11,  -1,  13,  -1,  -1,  -1,  17,  18,  -1,  20,  21,  22,  23,  -1,  25,  26,  27,  28

        };

        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                if(_m[i*n+j] > 0) {
                    rel(*this, m(i,j), IRT_EQ, _m[i*n+j], Gecode::ICL_DOM);
                }
            }
        }

        // we assume that the matrix is a square

        //int w = m.width();
        for(int i = 0; i < n; i++) {
            distinct(*this, m.row(i), Gecode::ICL_DOM);
            distinct(*this, m.col(i), Gecode::ICL_DOM);
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
    QuasiGroupsCompletion(bool share, QuasiGroupsCompletion& s) : MyFlatZincSpace(share, s), n(s.n) {}

    // Copy during cloning
    virtual Space*
    copy(bool share) {
        return new QuasiGroupsCompletion(share,*this);
    }
};

#endif