/*---------------------------------------------------------------------------*/
/*                                                                           */
/* magicsquares.h													    */
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

#ifndef __MODELS_MAGICSQUARE_H__
#define __MODELS_MAGICSQUARE_H__

#include "../flatzinc.h"
#include "../stl_util.h"

#include <list>

using namespace std;
using namespace Gecode;

class MagicSquare : public MyFlatZincSpace {
private:
    /// Size of magic square
    const int n;
    /// Fields of square
    //IntVarArray x;

public:

    static MyFlatZincSpace* getInstance(MyFlatZincOptions& opt) {

        string name_instance("magicsquare_" + stl_util::Convert2String(opt.nsize()));
        opt.name(name_instance.c_str());

        //opt.size(7);

        return new MagicSquare(opt);
    }

    /// Post constraints
    MagicSquare(const MyFlatZincOptions& opt)
        : n(opt.nsize()) {

        // Number of fields on square
        const int nn = n*n;

        iv = IntVarArray(*this,nn,1,nn);
        _method = SAT;
        _optVar = -1;
        intVarCount = iv.size();
        boolVarCount = setVarCount = floatVarCount = 0;


        // Sum of all a row, column, or diagonal
        const int s  = nn*(nn+1) / (2*n);

        // Matrix-wrapper for the square
        Matrix<IntVarArray> m(iv, n, n);

        for (int i = n; i--; ) {
            linear(*this, m.row(i), IRT_EQ, s, opt.icl());
            linear(*this, m.col(i), IRT_EQ, s, opt.icl());
        }
        // Both diagonals must have sum s
        {
            IntVarArgs d1y(n);
            IntVarArgs d2y(n);
            for (int i = n; i--; ) {
                d1y[i] = m(i,i);
                d2y[i] = m(n-i-1,i);
            }
            linear(*this, d1y, IRT_EQ, s, opt.icl());
            linear(*this, d2y, IRT_EQ, s, opt.icl());
        }

        // All fields must be distinct
        distinct(*this, iv, opt.icl());

        // Break some (few) symmetries
        rel(*this, m(0,0), IRT_GR, m(0,n-1));
        rel(*this, m(0,0), IRT_GR, m(n-1,0));

    }

    void createBranchers() {
        // branching
        branch(*this, iv, INT_VAR_SIZE_MIN(), INT_VAL_SPLIT_MIN());
    }

    /// Constructor for cloning \a s
    MagicSquare(bool share, MagicSquare& s) : MyFlatZincSpace(share,s), n(s.n) {
    }

    /// Copy during cloning
    virtual Space*
    copy(bool share) {
        return new MagicSquare(share,*this);
    }
    /// Print solution
    virtual void
    print(std::ostream& os) const {
        // Matrix-wrapper for the square
        Matrix<IntVarArray> m(iv, n, n);
        for (int i = 0; i<n; i++) {
            os << "\t";
            for (int j = 0; j<n; j++) {
                os.width(2);
                os << m(i,j) << "  ";
            }
            os << std::endl;
        }
    }

};

#endif