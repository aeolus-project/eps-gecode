/*---------------------------------------------------------------------------*/
/*                                                                           */
/* steelmill.h													    */
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

#ifndef __MODELS_STILLMILL_H__
#define __MODELS_STILLMILL_H__

#include "../flatzinc.h"
#include "../stl_util.h"

#include <list>

using namespace std;
using namespace Gecode;

/** \brief Order-specifications
 *
 * Used in the \ref SteelMill example.
 *
 */
//@{
typedef int (*order_t)[2];     ///< Type of the order-specification
extern const int order_weight; ///< Weight-position in order-array elements
extern const int order_color;  ///< Color-position in order-array elements
//@}

/** \brief Constants for CSPLib instance of the Steel Mill Slab Design Problem.
 *
 * Used in the \ref SteelMill example.
 */
//@{
extern int csplib_capacities[];         ///< Capacities
extern unsigned int csplib_ncapacities; ///< Number of capacities
extern unsigned int csplib_maxcapacity; ///< Maximum capacity
extern int csplib_loss[];               ///< Loss for all sizes
extern int csplib_orders[][2];          ///< Orders
extern unsigned int csplib_ncolors;     ///< Number of colors
extern unsigned int csplib_norders;     ///< Number of orders
//@}


/** \brief %SteelMillOptions for examples with size option and an additional
 * optional file name parameter.
 *
 * Used in the \ref SteelMill example.
 */
class SteelMillOptions : public Options {
private:
    unsigned int _size;    ///< Size value
    int* _capacities;      ///< Capacities
    int  _ncapacities;     ///< Number of capacities
    int  _maxcapacity;     ///< Maximum capacity
    int* _loss;            ///< Loss for all sizes
    order_t _orders;       ///< Orders
    int  _ncolors;         ///< Number of colors
    unsigned int _norders; ///< Number of orders
public:
    /// Initialize options for example with name \a n
    SteelMillOptions(const char* n)
        : Options(n), _size(csplib_norders),
          _capacities(csplib_capacities), _ncapacities(csplib_ncapacities),
          _maxcapacity(csplib_maxcapacity),
          _loss(csplib_loss), _orders(&(csplib_orders[0])), _ncolors(csplib_ncolors),
          _norders(csplib_norders) {

        // Compute loss
        _loss[0] = 0;
        int currcap = 0;
        for (int c = 1; c < _maxcapacity; ++c) {
            if (c > _capacities[currcap]) ++currcap;
            _loss[c] = _capacities[currcap] - c;
        }
    }
    /// Print help text
    virtual void help(void);
    /// Parse options from arguments \a argv (number is \a argc)
    bool parse(const char* filename);

    /// Return size
    unsigned int size(void) const {
        return _size;
    }
    /// Return capacities
    int* capacities(void) const   {
        return _capacities;
    }
    /// Return number of capacities
    int ncapacities(void) const   {
        return _ncapacities;
    }
    /// Return maximum of capacities
    int maxcapacity(void) const   {
        return _maxcapacity;
    }
    /// Return loss values
    int* loss(void) const         {
        return _loss;
    }
    /// Return orders
    order_t orders(void) const    {
        return _orders;
    }
    /// Return number of colors
    int ncolors(void) const       {
        return _ncolors;
    }
    /// Return number of orders
    int norders(void) const       {
        return _norders;
    }
};

/// Sort orders by weight
class SortByWeight {
public:
    /// The orders
    order_t orders;
    /// Initialize orders
    SortByWeight(order_t _orders) : orders(_orders) {}
    /// Sort order
    bool operator() (int i, int j) {
        // Order i comes before order j if the weight of i is larger than
        // the weight of j.
        return (orders[i][order_weight] > orders[j][order_weight]) ||
               (orders[i][order_weight] == orders[j][order_weight] && i<j);
    }
};

/**
 * \brief %Example: Steel-mill slab design problem
 *
 * This model solves the Steel Mill Slab Design Problem (Problem 38 in
 * <a href="http://csplib.org">CSPLib</a>). The model is from Gargani
 * and Refalo, "An efficient model and strategy for the steel mill
 * slab design problem.", CP 2007, except that a decomposition of the
 * packing constraint is used. The symmetry-breaking search is from
 * Van Hentenryck and Michel, "The Steel Mill Slab Design Problem
 * Revisited", CPAIOR 2008.
 *
 * The program accepts an optional argument for a data-file containing
 * an instance of the problem. The format for the data-file is the following:
 * <pre>
 * "number of slab capacities" "sequence of capacities in increasing order"
 * "number of colors"
 * "number of orders"
 * "size order 1" "color of order 1"
 * "size order 2" "color of order 2"
 * ...
 * </pre>
 * Hard instances are available from <a href=
 * "http://becool.info.ucl.ac.be/steelmillslab">
 * http://becool.info.ucl.ac.be/steelmillslab</a>.
 *
 * \ingroup Example
 *
 */
class SteelMill : public MyFlatZincSpace {
protected:
    /** \name Instance specification
     */
    //@{
    int* capacities;      ///< Capacities
    int  ncapacities;     ///< Number of capacities
    int  maxcapacity;     ///< Maximum capacity
    int* loss;            ///< Loss for all sizes
    int  ncolors;         ///< Number of colors
    order_t orders;       ///< Orders
    unsigned int norders; ///< Number of orders
    unsigned int nslabs;  ///< Number of slabs
    //@}

    /** \name Problem variables
     */
    //@{
    IntVarArray //slab, ///< Slab assigned to order i
    slabload, ///< Load of slab j
    slabcost; ///< Cost of slab j
    //IntVar total_cost; ///< Total cost
    //@}

public:
    /// Branching variants
    enum symmetry {
        SYMMETRY_NONE,      ///< Simple symmetry
        SYMMETRY_BRANCHING, ///< Breaking symmetries with symmetry
        SYMMETRY_LDSB       ///< Use LDSB for symmetry breaking
    } sym;


    static MyFlatZincSpace* getInstance(MyFlatZincOptions& opt, const char* filename) {

        SteelMillOptions optSteelMill("Steel Mill Slab design");
        optSteelMill.symmetry(SteelMill::SYMMETRY_BRANCHING);
        if(filename) {
            if(!optSteelMill.parse(filename)) {
                return NULL;
            }
        }
        /*
        opt.symmetry(SteelMill::SYMMETRY_NONE,"none");
        opt.symmetry(SteelMill::SYMMETRY_BRANCHING,"branching");
        opt.symmetry(SteelMill::SYMMETRY_LDSB,"ldsb");
        opt.solutions(0);
        */

        //opt.icl(ICL_BND);
        SteelMill* s = new SteelMill(optSteelMill);
        //s->print(std::cerr);
        //getchar();
        return s;
        //return new SteelMill(optSteelMill);
    }

    /// Actual model
    SteelMill(const SteelMillOptions& opt)
        : // Initialize instance data
        capacities(opt.capacities()), ncapacities(opt.ncapacities()),
        maxcapacity(opt.maxcapacity()), loss(opt.loss()),
        ncolors(opt.ncolors()), orders(opt.orders()),
        norders(opt.size()), nslabs(opt.size()),
        sym(static_cast<enum symmetry>(opt.symmetry())),
        // Initialize problem variables
        //slab(*this, norders, 0,nslabs-1),
        slabload(*this, nslabs, 0,45),
        slabcost(*this, nslabs, 0, Int::Limits::max)//,
        //total_cost(*this, 0, Int::Limits::max)
    {

        iv = IntVarArray(*this, norders+1);

        ///slab
        IntVarArgs slab;
        for(int i = 0; i < norders; i++) {
            iv[i] = IntVar(*this, 0,nslabs-1);
            slab << iv[i];
        }

        _method = MIN;
        _optVar = iv.size()-1;
        intVarCount = slabload.size() + slabcost.size() + iv.size();
        boolVarCount = setVarCount = floatVarCount = 0;

        /// Number of bins
        IntVarArgs total_cost;
        iv[_optVar] = IntVar(*this, 0, Int::Limits::max);
        total_cost << iv[_optVar];

        // Boolean variables for slab[o]==s
        BoolVarArgs boolslab(norders*nslabs);
        for (unsigned int i = 0; i < norders; ++i) {
            BoolVarArgs tmp(nslabs);
            for (int j = nslabs; j--; ) {
                boolslab[j + i*nslabs] = tmp[j] = BoolVar(*this, 0, 1);
            }
            channel(*this, tmp, slab[i]);
        }

        // Packing constraints
        for (unsigned int s = 0; s < nslabs; ++s) {
            IntArgs c(norders);
            BoolVarArgs x(norders);
            for (int i = norders; i--; ) {
                c[i] = orders[i][order_weight];
                x[i] = boolslab[s + i*nslabs];
            }
            linear(*this, c, x, IRT_EQ, slabload[s]);
        }
        // Redundant packing constraint
        int totalweight = 0;
        for (unsigned int i = norders; i-- ; )
            totalweight += orders[i][order_weight] ;
        linear(*this, slabload, IRT_EQ, totalweight);


        // Color constraints
        IntArgs nofcolor(ncolors);
        for (int c = ncolors; c--; ) {
            nofcolor[c] = 0;
            for (int o = norders; o--; ) {
                if (orders[o][order_color] == c) nofcolor[c] += 1;
            }
        }
        BoolVar f(*this, 0, 0);
        for (unsigned int s = 0; s < nslabs; ++s) {
            BoolVarArgs hascolor(ncolors);
            for (int c = ncolors; c--; ) {
                if (nofcolor[c]) {
                    BoolVarArgs hasc(nofcolor[c]);
                    int pos = 0;
                    for (int o = norders; o--; ) {
                        if (orders[o][order_color] == c)
                            hasc[pos++] = boolslab[s + o*nslabs];
                    }
                    assert(pos == nofcolor[c]);
                    hascolor[c] = BoolVar(*this, 0, 1);
                    rel(*this, BOT_OR, hasc, hascolor[c]);
                } else {
                    hascolor[c] = f;
                }
            }
            linear(*this, hascolor, IRT_LQ, 2);
        }

        // Compute slabcost
        IntArgs l(maxcapacity, loss);
        for (int s = nslabs; s--; ) {
            element(*this, l, slabload[s], slabcost[s]);
        }
        linear(*this, slabcost, IRT_EQ, iv[_optVar]);


    }

    void createBranchers() {

        ///slab
        IntVarArgs slab;
        for(int i = 0; i < norders; i++) {
            slab << iv[i];
        }

        /// Number of bins
        IntVarArgs total_cost;
        total_cost << iv[_optVar];

        // Add branching

        if (sym == SYMMETRY_BRANCHING) {
            // Symmetry breaking branching
            SteelMillBranch::post(*this);
        } else if (sym == SYMMETRY_NONE) {
            branch(*this, slab, INT_VAR_MAX_MIN(), INT_VAL_MIN());
        } else { // opt.symmetry() == SYMMETRY_LDSB
            // There is one symmetry: the values (slabs) are interchangeable.
            Symmetries syms;
            syms << ValueSymmetry(IntArgs::create(nslabs,0));

            // For variable order we mimic the custom brancher.  We use
            // min-size domain, breaking ties by maximum weight (preferring
            // to label larger weights earlier).  To do this, we first sort
            // (stably) by maximum weight, then use min-size domain.
            SortByWeight sbw(orders);
            IntArgs indices(norders);
            for (unsigned int i = 0 ; i < norders ; i++)
                indices[i] = i;
            Support::quicksort(&indices[0],norders,sbw);
            IntVarArgs sorted_orders(norders);
            for (unsigned int i = 0 ; i < norders ; i++) {
                sorted_orders[i] = slab[indices[i]];
            }
            branch(*this, sorted_orders, INT_VAR_SIZE_MIN(), INT_VAL_MIN(), syms);
        }
    }

    /// Print solution
    virtual void
    print(std::ostream& os) const {
        os << "What slab="  << iv << std::endl;
        os << "Slab load="  << slabload << std::endl;
        os << "Slab cost="  << slabcost << std::endl;
        os << "Total cost=" << iv[_optVar] << std::endl;
        int nslabsused = 0;
        int nslabscost = 0;
        bool unassigned = false;
        for (int i = nslabs; i--; ) {
            if (!slabload[i].assigned() || !slabcost[i].assigned()) {
                unassigned = true;
                break;
            }
            if (slabload[i].min()>0) ++nslabsused;
            if (slabcost[i].min()>0) ++nslabscost;
        }
        if (!unassigned)
            os << "Number of slabs used=" << nslabsused
               << ", slabs with cost="    << nslabscost
               << std::endl;
        os << std::endl;
    }

    /// Constructor for cloning \a s
    SteelMill(bool share, SteelMill& s)
        : MyFlatZincSpace(share,s),
          capacities(s.capacities), ncapacities(s.ncapacities),
          maxcapacity(s.maxcapacity), loss(s.loss),
          ncolors(s.ncolors), orders(s.orders),
          norders(s.norders), nslabs(s.nslabs) {
        //slab.update(*this, share, s.slab);
        slabload.update(*this, share, s.slabload);
        slabcost.update(*this, share, s.slabcost);
        //total_cost.update(*this, share, s.total_cost);
    }
    /// Copy during cloning
    virtual Space*
    copy(bool share) {
        return new SteelMill(share,*this);
    }
    /// Return solution cost
    /*
    virtual IntVar cost(void) const {
      return total_cost;
    }
    */

    /** \brief Custom brancher for steel mill slab design
     *
     * This class implements a custom brancher for SteelMill that
     * considers all slabs with no order assigned to it currently to be
     * symmetric.
     *
     * \relates SteelMill
     */
    class SteelMillBranch : Brancher {
    protected:
        /// Cache of first unassigned value
        mutable int start;
        /// %Choice
        class Choice : public Gecode::Choice {
        public:
            /// Position of variable
            int pos;
            /// Value of variable
            int val;
            /** Initialize choice for brancher \a b, number of
             *  alternatives \a a, position \a pos0, and value \a val0.
             */
            Choice(const Brancher& b, unsigned int a, int pos0, int val0)
                : Gecode::Choice(b,a), pos(pos0), val(val0) {}
            /// Report size occupied
            virtual size_t size(void) const {
                return sizeof(Choice);
            }
            /// Archive into \a e
            virtual void archive(Archive& e) const {
                Gecode::Choice::archive(e);
                e << alternatives() << pos << val;
            }
        };

        /// Construct brancher
        SteelMillBranch(Home home)
            : Brancher(home), start(0) {}
        /// Copy constructor
        SteelMillBranch(Space& home, bool share, SteelMillBranch& b)
            : Brancher(home, share, b), start(b.start) {
        }

    public:
        /// Check status of brancher, return true if alternatives left.
        virtual bool status(const Space& home) const {
            const SteelMill& sm = static_cast<const SteelMill&>(home);
            for (unsigned int i = start; i < sm.norders; ++i)
                if (!sm.iv[i].assigned()) {
                    start = i;
                    return true;
                }
            // No non-assigned orders left
            return false;
        }
        /// Return choice
        virtual Gecode::Choice* choice(Space& home) {
            SteelMill& sm = static_cast<SteelMill&>(home);
            //assert(!sm.slab[start].assigned());
            // Find order with a) minimum size, b) largest weight
            unsigned int size = sm.norders;
            int weight = 0;
            unsigned int pos = start;
            for (unsigned int i = start; i<sm.norders; ++i) {
                if (!sm.iv[i].assigned()) {
                    if (sm.iv[i].size() == size &&
                            sm.orders[i][order_weight] > weight) {
                        weight = sm.orders[i][order_weight];
                        pos = i;
                    } else if (sm.iv[i].size() < size) {
                        size = sm.iv[i].size();
                        weight = sm.orders[i][order_weight];
                        pos = i;
                    }
                }
            }
            unsigned int val = sm.iv[pos].min();
            // Find first still empty slab (all such slabs are symmetric)
            unsigned int firstzero = 0;
            while (firstzero < sm.nslabs && sm.slabload[firstzero].min() > 0)
                ++firstzero;
            assert(pos < sm.nslabs &&
                   val < sm.norders);
            return new Choice(*this, (val<firstzero) ? 2 : 1, pos, val);
        }
        virtual Choice* choice(const Space&, Archive& e) {
            unsigned int alt;
            int pos, val;
            e >> alt >> pos >> val;
            return new Choice(*this, alt, pos, val);
        }
        /// Perform commit for choice \a _c and alternative \a a
        virtual ExecStatus commit(Space& home, const Gecode::Choice& _c,
                                  unsigned int a) {
            SteelMill& sm = static_cast<SteelMill&>(home);
            const Choice& c = static_cast<const Choice&>(_c);
            if (a)
                return me_failed(Int::IntView(sm.iv[c.pos]).nq(home, c.val))
                       ? ES_FAILED : ES_OK;
            else
                return me_failed(Int::IntView(sm.iv[c.pos]).eq(home, c.val))
                       ? ES_FAILED : ES_OK;
        }
        /// Copy brancher
        virtual Actor* copy(Space& home, bool share) {
            return new (home) SteelMillBranch(home, share, *this);
        }
        /// Post brancher
        static BrancherHandle post(Home home) {
            return *new (home) SteelMillBranch(home);
        }
        /// Delete brancher and return its size
        virtual size_t dispose(Space&) {
            return sizeof(*this);
        }
    };
};

void
SteelMillOptions::help(void) {
    Options::help();
    std::cerr << "\t(string), optional" << std::endl
              << "\t\tBenchmark to load." << std::endl
              << "\t\tIf none is given, the standard CSPLib instance is used."
              << std::endl;
    std::cerr << "\t(unsigned int), optional" << std::endl
              << "\t\tNumber of orders to use, in the interval [0..norders]."
              << std::endl
              << "\t\tIf none is given, all orders are used." << std::endl;
}

bool
SteelMillOptions::parse(const char* filename) {
    // Check number of arguments

    std::ifstream instance(filename);
    if (instance.fail()) {
        std::cerr << "Argument \"" << filename
                  << "\" is not a readable file"
                  << std::endl;
        return false;
    }
    _size = 0;
    // Read file instance
    instance >> _ncapacities;
    _capacities = new int[_ncapacities];
    _maxcapacity = -1;
    for (int i = 0; i < _ncapacities; ++i) {
        instance >> _capacities[i];
        _maxcapacity = std::max(_maxcapacity, _capacities[i]);
    }
    instance >> _ncolors >> _norders;
    _orders = new int[_norders][2];
    for (unsigned int i = 0; i < _norders; ++i) {
        instance >> _orders[i][order_weight] >> _orders[i][order_color];
    }
    // Compute loss
    {
        _loss = new int[_maxcapacity+1];
        _loss[0] = 0;
        int currcap = 0;
        for (int c = 1; c < _maxcapacity; ++c) {
            if (c > _capacities[currcap]) ++currcap;
            _loss[c] = _capacities[currcap] - c;
        }
    }
    // Set size, if none given
    if (_size == 0) {
        _size = _norders;
    }
    // Check size reasonability
    if (_size == 0 || _size > _norders) {
        std::cerr << "Size must be between 1 and " << _norders << std::endl;
        return false;
    }
    return true;
}

// Positions in order array
const int order_weight = 0;
const int order_color = 1;

// CSPLib instance
int csplib_capacities[] = {
    11, 12, 13, 15, 16, 19, 24, 25, 27, 29, 31, 33, 34, 36, 39, 40, 41, 43, 46, 48
};
unsigned int csplib_ncapacities = 20;
unsigned int csplib_maxcapacity = 48;
int csplib_loss[49];
unsigned int csplib_ncolors = 89;
unsigned int csplib_norders = 111;
int csplib_orders[][2] = {

    {4, 1},
    {22, 2},
    {9, 3},
    {5, 4},
    {8, 5},
    {3, 6},
    {3, 4},
    {4, 7},
    {7, 4},
    {7, 8},
    {3, 6},
    {2, 6},
    {2, 4},
    {8, 9},
    {5, 0},
    {7, 1},
    {4, 7},
    {7, 1},
    {5, 0},
    {7, 1},
    {8, 9},
    {3, 1},
    {25, 2},
    {14, 3},
    {3, 6},
    {22, 4},
    {19, 5},
    {19, 5},
    {22, 6},
    {22, 7},
    {22, 8},
    {20, 9},
    {22, 0},
    {5, 1},
    {4, 2},
    {10, 3},
    {26, 4},
    {17, 5},
    {20, 6},
    {16, 7},
    {10, 8},
    {19, 9},
    {10, 0},
    {10, 1},
    {23, 2},
    {22, 3},
    {26, 4},
    {27, 5},
    {22, 6},
    {27, 7},
    {22, 8},
    {22, 9},
    {13, 0},
    {14, 1},
    {16, 7},
    {26, 4},
    {26, 2},
    {27, 5},
    {22, 6},
    {20, 3},
    {26, 4},
    {22, 4},
    {13, 5},
    {19, 6},
    {20, 7},
    {16, 8},
    {15, 9},
    {17, 0},
    {10, 8},
    {20, 1},
    {5, 2},
    {26, 4},
    {19, 3},
    {15, 4},
    {10, 5},
    {10, 6},
    {13, 7},
    {13, 8},
    {13, 9},
    {12, 0},
    {12, 1},
    {18, 2},
    {10, 3},
    {18, 4},
    {16, 5},
    {20, 6},
    {12, 7},
    {6, 8},
    {6, 8},
    {15, 9},
    {15, 0},
    {15, 0},
    {21, 1},
    {30, 2},
    {30, 3},
    {30, 4},
    {30, 5},
    {23, 6},
    {15, 7},
    {15, 8},
    {27, 9},
    {27, 0},
    {27, 1},
    {27, 2},
    {27, 3},
    {27, 4},
    {27, 9},
    {27, 5},
    {27, 6},
    {10, 7},
    {3, 8}
};


#endif