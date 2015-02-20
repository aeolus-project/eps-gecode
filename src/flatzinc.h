/*---------------------------------------------------------------------------*/
/*                                                                           */
/* flatzinc.h													    */
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

#ifndef __FLATZINC_H__
#define __FLATZINC_H__

#include <gecode/flatzinc.hh>
#include <string>


class MyFlatZincOptions : public Gecode::FlatZinc::FlatZincOptions {

protected:

    /// \name Model options
    //@{
    Gecode::Driver::StringOption     _model; ///< Model to use
    Gecode::Driver::StringOption _symmetry;    ///< General symmetry options
    Gecode::Driver::StringOption _propagation; ///< Propagation options
    Gecode::Driver::StringOption _icl;         ///< Integer consistency level
    Gecode::Driver::StringOption _branching;   ///< Branching options


    /// \name Search options
    Gecode::Driver::UnsignedIntOption _problems; ///< How many problems to generate
    Gecode::Driver::UnsignedIntOption _mode_decomposition; ///< Mode decomposition for eps
    Gecode::Driver::StringOption      _search; ///< Search engine variant
    Gecode::Driver::BoolOption        _add_ub; ///< Use upperbound
    Gecode::Driver::BoolOption        _add_lb; ///< Use lowerbound
    Gecode::Driver::IntOption        _ub; ///< Set upperbound
    Gecode::Driver::IntOption        _lb; ///< set lowerbound

    Gecode::Driver::UnsignedIntOption     _nsize; ///< like SizeOption
    Gecode::Driver::UnsignedIntOption     _msize; ///< like SizeOption

    Gecode::Driver::UnsignedIntOption     _nspf; ///< like SizeOption

    Gecode::Driver::StringValueOption      _sp_file;   ///< subproblem file path
    Gecode::Driver::StringValueOption      _obj_file;   ///< objective file path

    Gecode::Driver::UnsignedIntOption _first_level;

    Gecode::Driver::BoolOption _cobj;   ///< Branching options


    Gecode::Driver::StringValueOption     _dl; ///< like SizeOption

public:

    enum ModelOptions {
        MODEL_FLATZINC, //< model flatzinc
        MODEL_NQUEENS,
        MODEL_GOLOMBRULER,
        MODEL_ALLINTERVAL,
        MODEL_LATINSQUARES,
        MODEL_MAGICSQUARE,
        MODEL_PARTITION,
        MODEL_BACP,
        MODEL_GRAPHCOLOR,
        MODEL_BINPACKING,
        MODEL_SPORTSLEAGUE,
        MODEL_QUASIGROUPSCOMPLETION,
        MODEL_STEELMILL,
        MODEL_ENUMERATION
    };


    enum SearchOptions {
        FZ_SEARCH_BAB, //< Branch-and-bound search
        FZ_SEARCH_EPS, //< EPS search
        FZ_SEARCH_EPS_GRID_GENERATION, //< EPS search
        FZ_SEARCH_EPS_GRID_COMPUTATION //< EPS search

    };

    enum ModeDecomposition {
        SIMPLE = 0,    //< SIMPLE
        DBDFS = 1, //< DBDFS generation of ndi problems in sequential
        DBDFSwP = 2 //< DBDFSwP generation of ndi problems in parallel
    };

    MyFlatZincOptions(const char* s) : Gecode::FlatZinc::FlatZincOptions(s),

        _model("-model","model variants", MODEL_FLATZINC),
        _symmetry("-symmetry","symmetry variants"),
        _propagation("-propagation","propagation variants"),
        _icl("-icl","integer consistency level",Gecode::ICL_DEF),
        _branching("-branching","branching variants"),

        _search("-search","search engine variant", FZ_SEARCH_BAB),

        _problems("-problems","number of problems generated for eps", 50),
        _mode_decomposition("-mode_decomposition","mode decomposition for eps (0 = SIMPLE_DECOMPOSITION, 1 = DBDFS, 2 = DBDFSwP)", 1),

        _add_ub("-add_ub","add upperbound", false),
        _add_lb("-add_lb","add lowerbound", false),
        _ub("-ub","set upperbound if added", 0),
        _lb("-lb","set lowerbound if added", 0),
        _nsize("-nsize","set size n used in a model", 5),
        _msize("-msize","set size m used in a model", 5),
        _sp_file("-spf","subproblem file path"),
        _obj_file("-objf","objective file path"),

        _nspf("-nspf","number of files to generate by file generation", 1),

        _first_level("-firstlevel", "first level to stop", 0),
        _cobj("-cobj", "communicate the objective during the resolution", false),

        _dl("-dl","set levels for decomposition", "") {
        //add suboptions
        _model.add(MODEL_FLATZINC, "flatzinc");
        _model.add(MODEL_NQUEENS, "nqueens");
        _model.add(MODEL_GOLOMBRULER, "golombruler");
        _model.add(MODEL_ALLINTERVAL, "allinterval");
        _model.add(MODEL_LATINSQUARES, "latinsquares");
        _model.add(MODEL_MAGICSQUARE, "magicsquare");
        _model.add(MODEL_PARTITION, "partition");
        _model.add(MODEL_BACP, "bacp");
        _model.add(MODEL_GRAPHCOLOR, "graphcolor");
        _model.add(MODEL_BINPACKING, "binpacking");
        _model.add(MODEL_SPORTSLEAGUE, "sportsleague");
        _model.add(MODEL_QUASIGROUPSCOMPLETION, "quasigroupscompletion");
        _model.add(MODEL_STEELMILL, "steelmill");
        _model.add(MODEL_ENUMERATION, "enumeration");


        _icl.add(Gecode::ICL_DEF, "def");
        _icl.add(Gecode::ICL_VAL, "val");
        _icl.add(Gecode::ICL_BND, "bnd");
        _icl.add(Gecode::ICL_DOM, "dom");

        _search.add(FZ_SEARCH_BAB, "bab");
        _search.add(FZ_SEARCH_EPS, "eps");
        _search.add(FZ_SEARCH_EPS_GRID_GENERATION, "eps_grid_generation");
        _search.add(FZ_SEARCH_EPS_GRID_COMPUTATION, "eps_grid_computation");

        //add options
        add(_model);
        add(_symmetry);
        add(_propagation);
        add(_icl);
        add(_branching);

        add(_search);
        add(_problems);
        add(_mode_decomposition);

        add(_add_ub);
        add(_add_lb);
        add(_ub);
        add(_lb);

        add(_nsize);
        add(_msize);

        add(_sp_file);   ///< subproblem file path
        add(_obj_file);   ///< objective file path
        add(_first_level);
        add(_cobj);
        add(_nspf);

        add(_dl);
    }

    MyFlatZincOptions(const Gecode::FlatZinc::FlatZincOptions& o) : Gecode::FlatZinc::FlatZincOptions(o),
        _model("-model","model variants", MODEL_FLATZINC),
        _symmetry("-symmetry","symmetry variants"),
        _propagation("-propagation","propagation variants"),
        _icl("-icl","integer consistency level",Gecode::ICL_DEF),
        _branching("-branching","branching variants"),

        _search("-search","search engine variant", FZ_SEARCH_BAB),
        _problems("-problems","number of problems generated for eps", 50),
        _mode_decomposition("-mode_decomposition","mode decomposition for eps (0 = SIMPLE_DECOMPOSITION, 1 = DBDFS, 2 = DBDFSwP)", 1),

        _add_ub("-add_ub","add upperbound", false),
        _add_lb("-add_lb","add lowerbound", false),
        _ub("-ub","set upperbound if added", 0),
        _lb("-lb","set lowerbound if added", 0),

        _nsize("-nsize","set size n used in a model", 5),
        _msize("-msize","set size m used in a model", 5),

        _sp_file("-spf","subproblem file path"),
        _obj_file("-objf","objective file path"),

        _nspf("-nspf","number of files to generate by file generation", 1),

        _first_level("-firstlevel", "first level to stop", 0),

        _cobj("-cobj", "communicate the objective during the resolution", false),

        _dl("-dl","set levels for decomposition", "") {

        //add suboptions
        _model.add(MODEL_FLATZINC, "flatzinc");
        _model.add(MODEL_NQUEENS, "nqueens");
        _model.add(MODEL_GOLOMBRULER, "golombruler");
        _model.add(MODEL_ALLINTERVAL, "allinterval");
        _model.add(MODEL_LATINSQUARES, "latinsquares");
        _model.add(MODEL_MAGICSQUARE, "magicsquare");
        _model.add(MODEL_PARTITION, "partition");
        _model.add(MODEL_BACP, "bacp");
        _model.add(MODEL_GRAPHCOLOR, "graphcolor");
        _model.add(MODEL_BINPACKING, "binpacking");
        _model.add(MODEL_SPORTSLEAGUE, "sportsleague");
        _model.add(MODEL_QUASIGROUPSCOMPLETION, "quasigroupscompletion");
        _model.add(MODEL_STEELMILL, "steelmill");
        _model.add(MODEL_ENUMERATION, "enumeration");

        _icl.add(Gecode::ICL_DEF, "def");
        _icl.add(Gecode::ICL_VAL, "val");
        _icl.add(Gecode::ICL_BND, "bnd");
        _icl.add(Gecode::ICL_DOM, "dom");

        _search.add(FZ_SEARCH_BAB, "bab");
        _search.add(FZ_SEARCH_EPS, "eps");
        _search.add(FZ_SEARCH_EPS_GRID_GENERATION, "eps_grid_generation");
        _search.add(FZ_SEARCH_EPS_GRID_COMPUTATION, "eps_grid_computation");


        //add options
        add(_model);
        add(_symmetry);
        add(_propagation);
        add(_icl);
        add(_branching);

        add(_search);
        add(_problems);
        add(_mode_decomposition);

        add(_add_ub);
        add(_add_lb);
        add(_ub);
        add(_lb);

        add(_nsize);
        add(_msize);

        add(_sp_file);   ///< subproblem file path
        add(_obj_file);   ///< objective file path

        add(_first_level);
        add(_cobj);

        add(_dl);
        add(_nspf);
    }

    MyFlatZincOptions(const MyFlatZincOptions& o) : Gecode::FlatZinc::FlatZincOptions(o),

        _model(o._model),
        _symmetry(o._symmetry),
        _propagation(o._propagation),
        _icl(o._icl),
        _branching(o._branching),

        _search(o._search),
        _problems(o._problems),
        _mode_decomposition(o._mode_decomposition),
        _add_ub(o._add_ub),
        _add_lb(o._add_lb),
        _ub(o._ub),
        _lb(o._lb),
        _nsize(o._nsize),
        _msize(o._msize),
        _sp_file(o._sp_file),   ///< subproblem file path
        _obj_file(o._obj_file),   ///< objective file path

        _nspf(o._nspf),

        _first_level(o._first_level),

        _cobj(o._cobj),

        _dl(o._dl) {
    }

    //-- Model
    void parse(int& argc, char* argv[]) {
        Gecode::BaseOptions::parse(argc,argv);
        if (_stat.value())
            _mode.value(Gecode::SM_STAT);
    }

    inline int
    symmetry(void) const {
        return _symmetry.value();
    }

    inline void
    symmetry(int v) {
        _symmetry.value(v);
    }


    inline int
    propagation(void) const {
        return _propagation.value();
    }

    inline void
    propagation(int v) {
        _propagation.value(v);
    }

    inline Gecode::IntConLevel
    icl(void) const {
        return static_cast<Gecode::IntConLevel>(_icl.value());
    }

    inline void
    icl(Gecode::IntConLevel i) {
        _icl.value(i);
    }

    inline int
    branching(void) const {
        return _branching.value();
    }

    inline void
    branching(int v) {
        _branching.value(v);
    }


    ModelOptions model(void) const {
        return static_cast<ModelOptions>(_model.value());
    }

    //-- Search
    SearchOptions search(void) const {
        return static_cast<SearchOptions>(_search.value());
    }

    unsigned int nspf(void) const {
        return _nspf.value();
    }

    unsigned int problems(void) const {
        return _problems.value();
    }

    unsigned int mode_decomposition(void) const {
        return _mode_decomposition.value();
    }

    bool add_ub(void) const {
        return _add_ub.value();
    }

    bool add_lb(void) const {
        return _add_lb.value();
    }

    int ub(void) const {
        return _ub.value();
    }

    int lb(void) const {
        return _lb.value();
    }

    unsigned int nsize(void) const {
        return _nsize.value();
    }

    unsigned int msize(void) const {
        return _msize.value();
    }

    const char* sp_file(void) const {
        return _sp_file.value();
    }
    const char* obj_file(void) const {
        return _obj_file.value();
    }

    unsigned int firstLevel(void) const {
        return _first_level.value();
    }

    inline bool
    cobj(void) {
        return _cobj.value();
    }

    const char* dl(void) const {
        return _dl.value();
    }


    ~MyFlatZincOptions() {}
};


class MyFlatZincSpace : public Gecode::FlatZinc::FlatZincSpace {

public:
    MyFlatZincSpace* _space_hook;
    unsigned int _time_decomposition;
    unsigned int _problems;
    unsigned int _depth_decomposition;
    unsigned int _iterations_decomposition;
    unsigned int _nodes_decomposition;
    unsigned int _fails_decomposition;
    unsigned int _memory_decomposition;
    unsigned int _time_max_inactivity;
    std::vector< std::vector<unsigned int> >* _time_subproblems_workers;
    std::string* _name_instance;
    /*
    /// The integer variables
    Gecode::IntVarArgs iv;

    /// The Boolean variables
    Gecode::BoolVarArgs bv;
    #ifdef GECODE_HAS_SET_VARS
    /// The set variables
    Gecode::SetVarArgs sv_decision;
    #endif
    #ifdef GECODE_HAS_FLOAT_VARS
    /// The float decision variables
    Gecode::FloatVarArgs fv_decision;
    #endif
    */
    /**
    * \brief Create branchers corresponding to the solve item annotations
    *
    * If \a ignoreUnknown is true, unknown solve item annotations will be
    * ignored, otherwise a warning is written to \a err.
    *
    * The seed for random branchers is given by the \a seed parameter.
    *
    */

    //Do vector< ... > for several branchements !!!!!!!!!!!!!!!!!!!!!!
    //it\E9rer dessus aussi
    /*
    std::vector< Gecode::TieBreak<Gecode::IntVarBranch> > branch_var_iv;
    std::vector< Gecode::IntValBranch > branch_val_iv;
    std::vector< Gecode::IntVarArgs > args_iv;

    std::vector< Gecode::TieBreak<Gecode::IntVarBranch> > branch_var_bv;
    std::vector< Gecode::IntValBranch > branch_val_bv;
    std::vector< Gecode::BoolVarArgs > args_bv;

    #ifdef GECODE_HAS_SET_VARS
    std::vector< Gecode::SetVarBranch > branch_var_sv;
    std::vector< Gecode::SetValBranch > branch_val_sv;
    std::vector< Gecode::SetVarArgs > args_sv;
    #endif

    #ifdef GECODE_HAS_FLOAT_VARS
    std::vector< Gecode::TieBreak<Gecode::FloatVarBranch> > branch_var_fv;
    std::vector< Gecode::FloatValBranch > branch_val_fv;
    std::vector< Gecode::FloatVarArgs > args_fv;
    #endif
    */
    /// Construct empty space
    MyFlatZincSpace(void) : FlatZincSpace(), _space_hook(NULL),
        _time_decomposition(0),
        _problems(0),
        _depth_decomposition(0),
        _iterations_decomposition(0),
        _nodes_decomposition(0),
        _fails_decomposition(0),
        _memory_decomposition(0),
        _time_max_inactivity(0),
        _time_subproblems_workers(NULL),
        _name_instance(NULL)
        //,filter_iv(NULL), filter_bv(NULL), filter_sv(NULL), filter_fv(NULL)
    {

    }

    /// Destructor
    ~MyFlatZincSpace() {
        if(_space_hook) {
            delete _space_hook;
        }
    }


    void constrain(const Space& s);

    void run(std::ostream& out, const std::string& name_instance, const Gecode::FlatZinc::Printer& p,
             const MyFlatZincOptions& opt, Gecode::Support::Timer& t_total);


    void createBranchers(Gecode::FlatZinc::AST::Node* ann, int seed, double decay,
                         bool ignoreUnknown,
                         std::ostream& err);

    virtual Gecode::Space*
    copy(bool share = true);

    void sortVariables(bool less = true);

    virtual void createBranchers() {}

protected:
    MyFlatZincSpace(bool share, MyFlatZincSpace& f);


    /// Run the search engine
    template<template<class> class Engine>
    void
    runEngine(std::ostream& out, const std::string& name_instance, const Gecode::FlatZinc::Printer& p,
              const MyFlatZincOptions& opt, Gecode::Support::Timer& t_total);

    /// Run the meta search engine
    template<template<class> class Engine,
             template<template<class> class,class> class Meta>
    void
    runMeta(std::ostream& out, const std::string& name_instance, const Gecode::FlatZinc::Printer& p,
            const MyFlatZincOptions& opt, Gecode::Support::Timer& t_total);
};

#endif
