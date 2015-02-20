/*---------------------------------------------------------------------------*/
/*                                                                           */
/* partition.h													    */
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

#ifndef __MODELS_PARTITION_H__
#define __MODELS_PARTITION_H__

#include "../flatzinc.h"
#include "../stl_util.h"

#include <list>

using namespace std;
using namespace Gecode;

/**
 * \brief %Example: partition numbers into two groups
 *
 * \ingroup Example
 */
class Partition : public MyFlatZincSpace {
protected:
    /// First group of numbers
    //IntVarArray x;
    /// Second group of numbers
    //IntVarArray y;
    int n;
public:
    int decay;

    static MyFlatZincSpace* getInstance(MyFlatZincOptions& opt) {

        string name_instance("partition_" + stl_util::Convert2String(opt.nsize()));
        opt.name(name_instance.c_str());

        opt.icl(ICL_BND);

        /*
        opt.size(32);
        opt.icl(ICL_BND);
        */

        return new Partition(opt);
    }

    /// Actual model
    Partition(const MyFlatZincOptions& opt)
        : MyFlatZincSpace(),
          n(opt.nsize()),
          decay(opt.decay())
          //,
          //x(*this,n,1,2*n),
          //y(*this,n,1,2*n)
    {

        iv = IntVarArray(*this,n*2);
        _method = SAT;
        _optVar = -1;
        intVarCount = iv.size();
        boolVarCount = setVarCount = floatVarCount = 0;

        /// First group of numbers
        IntVarArgs x;
        /// Second group of numbers
        IntVarArgs y;
        for(int i = 0; i < n; i++) {
            iv[i] = IntVar(*this, 1, 2*n);
            iv[n+i] = IntVar(*this, 1, 2*n);
            x << iv[i];
            y << iv[n+i];
        }

        // Break symmetries by ordering numbers in each group
        rel(*this, x, IRT_LE);
        rel(*this, y, IRT_LE);

        rel(*this, x[0], IRT_LE, y[0]);

        IntVarArgs xy(2*n);
        for (int i = n; i--; ) {
            xy[i] = x[i];
            xy[n+i] = y[i];
        }
        distinct(*this, xy, opt.icl());

        IntArgs c(2*n);
        for (int i = n; i--; ) {
            c[i] = 1;
            c[n+i] = -1;
        }
        linear(*this, c, xy, IRT_EQ, 0);

        // Array of products
        IntVarArgs sxy(2*n), sx(n), sy(n);

        for (int i = n; i--; ) {
            sx[i] = sxy[i] =   expr(*this, sqr(x[i]));
            sy[i] = sxy[n+i] = expr(*this, sqr(y[i]));
        }
        linear(*this, c, sxy, IRT_EQ, 0);

        // Redundant constraints
        linear(*this, x, IRT_EQ, 2*n*(2*n+1)/4);
        linear(*this, y, IRT_EQ, 2*n*(2*n+1)/4);
        linear(*this, sx, IRT_EQ, 2*n*(2*n+1)*(4*n+1)/12);
        linear(*this, sy, IRT_EQ, 2*n*(2*n+1)*(4*n+1)/12);

    }

    void createBranchers() {
        // branching
        /*
        IntVarArgs xy(2*n);
        for (int i = n; i--; ) {
            xy[i] = x[i];
            xy[n+i] = y[i];
        }
        */
        branch(*this, iv, INT_VAR_AFC_SIZE_MAX(decay), INT_VAL_MIN());
        //branch(*this, xy, INT_VAR_NONE(), INT_VAL_MAX());
        //branch(*this, iv, INT_VAR_NONE(), INT_VAL_MAX());
    }


    /// Constructor used during cloning \a s
    Partition(bool share, Partition& s) : MyFlatZincSpace(share,s), n(s.n) {
        //x.update(*this, share, s.x);
        //y.update(*this, share, s.y);
    }
    /// Copying during cloning
    virtual Space*
    copy(bool share) {
        return new Partition(share,*this);
    }
    /// Print solution
    virtual void
    print(std::ostream& os) const {
        os << "\t";
        int a, b;
        a = b = 0;
        for (int i = 0; i < iv.size() / 2; i++) {
            a += iv[i].val();
            b += iv[i].val()*iv[i].val();
            os << iv[i] << ", ";
        }
        os << " = " << a << ", " << b << std::endl << "\t";
        a = b = 0;
        for (int i = iv.size() / 2; i < iv.size(); i++) {
            a += iv[i].val();
            b += iv[i].val()*iv[i].val();
            os << iv[i] << ", ";
        }
        os << " = " << a << ", " << b << std::endl;
    }
};

#endif