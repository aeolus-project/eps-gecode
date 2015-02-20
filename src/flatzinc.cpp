#include "flatzinc.h"

#include <gecode/flatzinc/registry.hh>
#include <gecode/flatzinc/plugin.hh>

#include <gecode/search.hh>

#include "search.h"
#include "stl_util.h"

#include <vector>
#include <string>
using namespace std;
using namespace Gecode;
using namespace FlatZinc;

/**
 * \brief Branching on the introduced variables
 *
 * This brancher makes sure that when a solution is found for the model
 * variables, all introduced variables are either assigned, or the solution
 * can be extended to a solution of the introduced variables.
 *
 * The advantage over simply branching over the introduced variables is that
 * only one such extension will be searched for, instead of enumerating all
 * possible (equivalent) extensions.
 *
 */
class AuxVarBrancher : public Brancher {
protected:
    /// Flag whether brancher is done
    bool done;
    /// Construct brancher
    AuxVarBrancher(Home home, TieBreak<IntVarBranch> int_varsel0,
                   IntValBranch int_valsel0,
                   TieBreak<IntVarBranch> bool_varsel0,
                   IntValBranch bool_valsel0
#ifdef GECODE_HAS_SET_VARS
                   ,
                   SetVarBranch set_varsel0,
                   SetValBranch set_valsel0
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                   ,
                   TieBreak<FloatVarBranch> float_varsel0,
                   FloatValBranch float_valsel0
#endif
                  )
        : Brancher(home), done(false),
          int_varsel(int_varsel0), int_valsel(int_valsel0),
          bool_varsel(bool_varsel0), bool_valsel(bool_valsel0)
#ifdef GECODE_HAS_SET_VARS
          , set_varsel(set_varsel0), set_valsel(set_valsel0)
#endif
#ifdef GECODE_HAS_FLOAT_VARS
          , float_varsel(float_varsel0), float_valsel(float_valsel0)
#endif
    {}
    /// Copy constructor
    AuxVarBrancher(Space& home, bool share, AuxVarBrancher& b)
        : Brancher(home, share, b), done(b.done) {}

    /// %Choice that only signals failure or success
    class Choice : public Gecode::Choice {
    public:
        /// Whether brancher should fail
        bool fail;
        /// Initialize choice for brancher \a b
        Choice(const Brancher& b, bool fail0)
            : Gecode::Choice(b,1), fail(fail0) {}
        /// Report size occupied
        virtual size_t size(void) const {
            return sizeof(Choice);
        }
        /// Archive into \a e
        virtual void archive(Archive& e) const {
            Gecode::Choice::archive(e);
            e.put(fail);
        }
    };

    TieBreak<IntVarBranch> int_varsel;
    IntValBranch int_valsel;
    TieBreak<IntVarBranch> bool_varsel;
    IntValBranch bool_valsel;
#ifdef GECODE_HAS_SET_VARS
    SetVarBranch set_varsel;
    SetValBranch set_valsel;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
    TieBreak<FloatVarBranch> float_varsel;
    FloatValBranch float_valsel;
#endif

public:
    /// Check status of brancher, return true if alternatives left.
    virtual bool status(const Space& _home) const {
        if (done) return false;
        const FlatZincSpace& home = static_cast<const FlatZincSpace&>(_home);
        for (int i=0; i<home.iv_aux.size(); i++)
            if (!home.iv_aux[i].assigned()) return true;
        for (int i=0; i<home.bv_aux.size(); i++)
            if (!home.bv_aux[i].assigned()) return true;
#ifdef GECODE_HAS_SET_VARS
        for (int i=0; i<home.sv_aux.size(); i++)
            if (!home.sv_aux[i].assigned()) return true;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        for (int i=0; i<home.fv_aux.size(); i++)
            if (!home.fv_aux[i].assigned()) return true;
#endif
        // No non-assigned variables left
        return false;
    }
    /// Return choice
    virtual Choice* choice(Space& home) {
        done = true;
        FlatZincSpace& fzs = static_cast<FlatZincSpace&>(*home.clone());
        fzs.needAuxVars = false;
        branch(fzs,fzs.iv_aux,int_varsel,int_valsel);
        branch(fzs,fzs.bv_aux,bool_varsel,bool_valsel);
#ifdef GECODE_HAS_SET_VARS
        branch(fzs,fzs.sv_aux,set_varsel,set_valsel);
#endif
#ifdef GECODE_HAS_FLOAT_VARS
        branch(fzs,fzs.fv_aux,float_varsel,float_valsel);
#endif
        Search::Options opt;
        opt.clone = false;
        FlatZincSpace* sol = dfs(&fzs, opt);
        if (sol) {
            delete sol;
            return new Choice(*this,false);
        } else {
            return new Choice(*this,true);
        }
    }
    /// Return choice
    virtual Choice* choice(const Space&, Archive& e) {
        bool fail;
        e >> fail;
        return new Choice(*this, fail);
    }
    /// Perform commit for choice \a c
    virtual ExecStatus commit(Space&, const Gecode::Choice& c, unsigned int) {
        return static_cast<const Choice&>(c).fail ? ES_FAILED : ES_OK;
    }
    /// Print explanation
    virtual void print(const Space&, const Gecode::Choice& c,
                       unsigned int,
                       std::ostream& o) const {
        o << "FlatZinc("
          << (static_cast<const Choice&>(c).fail ? "fail" : "ok")
          << ")";
    }
    /// Copy brancher
    virtual Actor* copy(Space& home, bool share) {
        return new (home) AuxVarBrancher(home, share, *this);
    }
    /// Post brancher
    static void post(Home home,
                     TieBreak<IntVarBranch> int_varsel,
                     IntValBranch int_valsel,
                     TieBreak<IntVarBranch> bool_varsel,
                     IntValBranch bool_valsel
#ifdef GECODE_HAS_SET_VARS
                     ,
                     SetVarBranch set_varsel,
                     SetValBranch set_valsel
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                     ,
                     TieBreak<FloatVarBranch> float_varsel,
                     FloatValBranch float_valsel
#endif
                    ) {
        (void) new (home) AuxVarBrancher(home, int_varsel, int_valsel,
                                         bool_varsel, bool_valsel
#ifdef GECODE_HAS_SET_VARS
                                         , set_varsel, set_valsel
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                                         , float_varsel, float_valsel
#endif
                                        );
    }
    /// Delete brancher and return its size
    virtual size_t dispose(Space&) {
        return sizeof(*this);
    }
};

template<class Var>
void varValPrint(const Space &home, const BrancherHandle& bh,
                 unsigned int a,
                 Var, int i, const int& n,
                 std::ostream& o) {
    static_cast<const FlatZincSpace&>(home).branchInfo.print(bh,a,i,n,o);
}

#ifdef GECODE_HAS_FLOAT_VARS
void varValPrintF(const Space &home, const BrancherHandle& bh,
                  unsigned int a,
                  FloatVar, int i, const FloatNumBranch& nl,
                  std::ostream& o) {
    static_cast<const FlatZincSpace&>(home).branchInfo.print(bh,a,i,nl,o);
}
#endif

IntSet vs2is(IntVarSpec* vs) {
    if (vs->assigned) {
        return IntSet(vs->i,vs->i);
    }
    if (vs->domain()) {
        AST::SetLit* sl = vs->domain.some();
        if (sl->interval) {
            return IntSet(sl->min, sl->max);
        } else {
            int* newdom = heap.alloc<int>(static_cast<unsigned long int>(sl->s.size()));
            for (int i=sl->s.size(); i--;)
                newdom[i] = sl->s[i];
            IntSet ret(newdom, sl->s.size());
            heap.free(newdom, static_cast<unsigned long int>(sl->s.size()));
            return ret;
        }
    }
    return IntSet(Int::Limits::min, Int::Limits::max);
}

int vs2bsl(BoolVarSpec* bs) {
    if (bs->assigned) {
        return bs->i;
    }
    if (bs->domain()) {
        AST::SetLit* sl = bs->domain.some();
        assert(sl->interval);
        return std::min(1, std::max(0, sl->min));
    }
    return 0;
}

int vs2bsh(BoolVarSpec* bs) {
    if (bs->assigned) {
        return bs->i;
    }
    if (bs->domain()) {
        AST::SetLit* sl = bs->domain.some();
        assert(sl->interval);
        return std::max(0, std::min(1, sl->max));
    }
    return 1;
}

TieBreak<IntVarBranch> ann2ivarsel(AST::Node* ann, Rnd rnd, double decay) {
    if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
        if (s->id == "input_order")
            return TieBreak<IntVarBranch>(INT_VAR_NONE());
        if (s->id == "first_fail")
            return TieBreak<IntVarBranch>(INT_VAR_SIZE_MIN());
        if (s->id == "anti_first_fail")
            return TieBreak<IntVarBranch>(INT_VAR_SIZE_MAX());
        if (s->id == "smallest")
            return TieBreak<IntVarBranch>(INT_VAR_MIN_MIN());
        if (s->id == "largest")
            return TieBreak<IntVarBranch>(INT_VAR_MAX_MAX());
        if (s->id == "occurrence")
            return TieBreak<IntVarBranch>(INT_VAR_DEGREE_MAX());
        if (s->id == "max_regret")
            return TieBreak<IntVarBranch>(INT_VAR_REGRET_MIN_MAX());
        if (s->id == "most_constrained")
            return TieBreak<IntVarBranch>(INT_VAR_SIZE_MIN(),
                                          INT_VAR_DEGREE_MAX());
        if (s->id == "random") {
            return TieBreak<IntVarBranch>(INT_VAR_RND(rnd));
        }
        if (s->id == "afc_min")
            return TieBreak<IntVarBranch>(INT_VAR_AFC_MIN(decay));
        if (s->id == "afc_max")
            return TieBreak<IntVarBranch>(INT_VAR_AFC_MAX(decay));
        if (s->id == "afc_size_min")
            return TieBreak<IntVarBranch>(INT_VAR_AFC_SIZE_MIN(decay));
        if (s->id == "afc_size_max") {
            return TieBreak<IntVarBranch>(INT_VAR_AFC_SIZE_MAX(decay));
        }
        if (s->id == "activity_min")
            return TieBreak<IntVarBranch>(INT_VAR_ACTIVITY_MIN(decay));
        if (s->id == "activity_max")
            return TieBreak<IntVarBranch>(INT_VAR_ACTIVITY_MAX(decay));
        if (s->id == "activity_size_min")
            return TieBreak<IntVarBranch>(INT_VAR_ACTIVITY_SIZE_MIN(decay));
        if (s->id == "activity_size_max")
            return TieBreak<IntVarBranch>(INT_VAR_ACTIVITY_SIZE_MAX(decay));
    }
    std::cerr << "Warning, ignored search annotation: ";
    ann->print(std::cerr);
    std::cerr << std::endl;
    return TieBreak<IntVarBranch>(INT_VAR_NONE());
}

IntValBranch ann2ivalsel(AST::Node* ann, std::string& r0, std::string& r1,
                         Rnd rnd) {
    if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
        if (s->id == "indomain_min") {
            r0 = "=";
            r1 = "!=";
            return INT_VAL_MIN();
        }
        if (s->id == "indomain_max") {
            r0 = "=";
            r1 = "!=";
            return INT_VAL_MAX();
        }
        if (s->id == "indomain_median") {
            r0 = "=";
            r1 = "!=";
            return INT_VAL_MED();
        }
        if (s->id == "indomain_split") {
            r0 = "<=";
            r1 = ">";
            return INT_VAL_SPLIT_MIN();
        }
        if (s->id == "indomain_reverse_split") {
            r0 = ">";
            r1 = "<=";
            return INT_VAL_SPLIT_MAX();
        }
        if (s->id == "indomain_random") {
            r0 = "=";
            r1 = "!=";
            return INT_VAL_RND(rnd);
        }
        if (s->id == "indomain") {
            r0 = "=";
            r1 = "=";
            return INT_VALUES_MIN();
        }
        if (s->id == "indomain_middle") {
            std::cerr << "Warning, replacing unsupported annotation "
                      << "indomain_middle with indomain_median" << std::endl;
            r0 = "=";
            r1 = "!=";
            return INT_VAL_MED();
        }
        if (s->id == "indomain_interval") {
            std::cerr << "Warning, replacing unsupported annotation "
                      << "indomain_interval with indomain_split" << std::endl;
            r0 = "<=";
            r1 = ">";
            return INT_VAL_SPLIT_MIN();
        }
    }
    std::cerr << "Warning, ignored search annotation: ";
    ann->print(std::cerr);
    std::cerr << std::endl;
    r0 = "=";
    r1 = "!=";
    return INT_VAL_MIN();
}

IntAssign ann2asnivalsel(AST::Node* ann, Rnd rnd) {
    if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
        if (s->id == "indomain_min")
            return INT_ASSIGN_MIN();
        if (s->id == "indomain_max")
            return INT_ASSIGN_MAX();
        if (s->id == "indomain_median")
            return INT_ASSIGN_MED();
        if (s->id == "indomain_random") {
            return INT_ASSIGN_RND(rnd);
        }
    }
    std::cerr << "Warning, ignored search annotation: ";
    ann->print(std::cerr);
    std::cerr << std::endl;
    return INT_ASSIGN_MIN();
}

#ifdef GECODE_HAS_SET_VARS
SetVarBranch ann2svarsel(AST::Node* ann, Rnd rnd, double decay) {
    if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
        if (s->id == "input_order")
            return SET_VAR_NONE();
        if (s->id == "first_fail")
            return SET_VAR_SIZE_MIN();
        if (s->id == "anti_first_fail")
            return SET_VAR_SIZE_MAX();
        if (s->id == "smallest")
            return SET_VAR_MIN_MIN();
        if (s->id == "largest")
            return SET_VAR_MAX_MAX();
        if (s->id == "afc_min")
            return SET_VAR_AFC_MIN(decay);
        if (s->id == "afc_max")
            return SET_VAR_AFC_MAX(decay);
        if (s->id == "afc_size_min")
            return SET_VAR_AFC_SIZE_MIN(decay);
        if (s->id == "afc_size_max")
            return SET_VAR_AFC_SIZE_MAX(decay);
        if (s->id == "activity_min")
            return SET_VAR_ACTIVITY_MIN(decay);
        if (s->id == "activity_max")
            return SET_VAR_ACTIVITY_MAX(decay);
        if (s->id == "activity_size_min")
            return SET_VAR_ACTIVITY_SIZE_MIN(decay);
        if (s->id == "activity_size_max")
            return SET_VAR_ACTIVITY_SIZE_MAX(decay);
        if (s->id == "random") {
            return SET_VAR_RND(rnd);
        }
    }
    std::cerr << "Warning, ignored search annotation: ";
    ann->print(std::cerr);
    std::cerr << std::endl;
    return SET_VAR_NONE();
}

SetValBranch ann2svalsel(AST::Node* ann, std::string r0, std::string r1,
                         Rnd rnd) {
    (void) rnd;
    if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
        if (s->id == "indomain_min") {
            r0 = "in";
            r1 = "not in";
            return SET_VAL_MIN_INC();
        }
        if (s->id == "indomain_max") {
            r0 = "in";
            r1 = "not in";
            return SET_VAL_MAX_INC();
        }
        if (s->id == "outdomain_min") {
            r1 = "in";
            r0 = "not in";
            return SET_VAL_MIN_EXC();
        }
        if (s->id == "outdomain_max") {
            r1 = "in";
            r0 = "not in";
            return SET_VAL_MAX_EXC();
        }
    }
    std::cerr << "Warning, ignored search annotation: ";
    ann->print(std::cerr);
    std::cerr << std::endl;
    r0 = "in";
    r1 = "not in";
    return SET_VAL_MIN_INC();
}
#endif

#ifdef GECODE_HAS_FLOAT_VARS
TieBreak<FloatVarBranch> ann2fvarsel(AST::Node* ann, Rnd rnd,
                                     double decay) {
    if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
        if (s->id == "input_order")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_NONE());
        if (s->id == "first_fail")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_SIZE_MIN());
        if (s->id == "anti_first_fail")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_SIZE_MAX());
        if (s->id == "smallest")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_MIN_MIN());
        if (s->id == "largest")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_MAX_MAX());
        if (s->id == "occurrence")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_DEGREE_MAX());
        if (s->id == "most_constrained")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_SIZE_MIN(),
                                            FLOAT_VAR_DEGREE_MAX());
        if (s->id == "random") {
            return TieBreak<FloatVarBranch>(FLOAT_VAR_RND(rnd));
        }
        if (s->id == "afc_min")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_AFC_MIN(decay));
        if (s->id == "afc_max")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_AFC_MAX(decay));
        if (s->id == "afc_size_min")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_AFC_SIZE_MIN(decay));
        if (s->id == "afc_size_max")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_AFC_SIZE_MAX(decay));
        if (s->id == "activity_min")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_ACTIVITY_MIN(decay));
        if (s->id == "activity_max")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_ACTIVITY_MAX(decay));
        if (s->id == "activity_size_min")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_ACTIVITY_SIZE_MIN(decay));
        if (s->id == "activity_size_max")
            return TieBreak<FloatVarBranch>(FLOAT_VAR_ACTIVITY_SIZE_MAX(decay));
    }
    std::cerr << "Warning, ignored search annotation: ";
    ann->print(std::cerr);
    std::cerr << std::endl;
    return TieBreak<FloatVarBranch>(FLOAT_VAR_NONE());
}

FloatValBranch ann2fvalsel(AST::Node* ann, std::string r0, std::string r1) {
    if (AST::Atom* s = dynamic_cast<AST::Atom*>(ann)) {
        if (s->id == "indomain_split") {
            r0 = "<=";
            r1 = ">";
            return FLOAT_VAL_SPLIT_MIN();
        }
        if (s->id == "indomain_reverse_split") {
            r1 = "<=";
            r0 = ">";
            return FLOAT_VAL_SPLIT_MAX();
        }
    }
    std::cerr << "Warning, ignored search annotation: ";
    ann->print(std::cerr);
    std::cerr << std::endl;
    r0 = "<=";
    r1 = ">";
    return FLOAT_VAL_SPLIT_MIN();
}

#endif



//flatten annotation to expand in vector
void flattenAnnotations(AST::Array* ann, std::vector<AST::Node*>& out) {
    for (unsigned int i=0; i<ann->a.size(); i++) {
        if (ann->a[i]->isCall("seq_search")) {
            AST::Call* c = ann->a[i]->getCall();
            if (c->args->isArray())
                flattenAnnotations(c->args->getArray(), out);
            else
                out.push_back(c->args);
        } else {
            out.push_back(ann->a[i]);
        }
    }
}


void MyFlatZincSpace::run(std::ostream& out, const std::string& name_instance, const FlatZinc::Printer& p,
                          const MyFlatZincOptions& opt, Support::Timer& t_total) {
    switch (_method) {
    case MIN:
    case MAX:
        if (opt.search() == MyFlatZincOptions::FZ_SEARCH_EPS
                || opt.search() == MyFlatZincOptions::FZ_SEARCH_EPS_GRID_GENERATION) {
            //BAB EPS
            runEngine<EPS_BAB>(out,name_instance,p,opt,t_total);
        } else {
            //BAB
            runEngine<BAB>(out,name_instance,p,opt,t_total);
        }
        break;
    case SAT:
        if (opt.search() == MyFlatZincOptions::FZ_SEARCH_EPS
                || opt.search() == MyFlatZincOptions::FZ_SEARCH_EPS_GRID_GENERATION) {
            //DFS EPS THREADS
            runEngine<EPS_DFS>(out,name_instance,p,opt,t_total);
        } else {
            //DFS
            runEngine<DFS>(out,name_instance,p,opt,t_total);
        }
        break;
    }
}

template<template<class> class Engine>
void
MyFlatZincSpace::runEngine(std::ostream& out, const std::string& name_instance, const Gecode::FlatZinc::Printer& p,
                           const MyFlatZincOptions& opt, Support::Timer& t_total) {
    if (opt.restart()==RM_NONE) {
        runMeta<Engine,Driver::EngineToMeta>(out,name_instance,p,opt,t_total);
    } else {
        runMeta<Engine,RBS>(out,name_instance,p,opt,t_total);
    }
}

/// Run the meta search engine
template<template<class> class Engine,
         template<template<class> class,class> class Meta>
void
MyFlatZincSpace::runMeta(std::ostream& out, const std::string& name_instance, const Gecode::FlatZinc::Printer& p,
                         const MyFlatZincOptions& opt, Support::Timer& t_total) {

#ifdef GECODE_HAS_GIST
    if (opt.mode() == SM_GIST) {
        FZPrintingInspector<FlatZincSpace> pi(p);
        FZPrintingComparator<FlatZincSpace> pc(p);
        (void) GistEngine<Engine<FlatZincSpace> >::explore(this,opt,&pi,&pc);
        return;
    }
#endif
    StatusStatistics sstat;
    unsigned int n_p = 0;
    Support::Timer t_solve;
    t_solve.start();
    if (status(sstat) != SS_FAILED) {
        n_p = propagators();
    }

    this->_name_instance = new std::string(name_instance);

    int initial_objective = -1;
    if(_method != SAT) {
        if(_method == MIN) {
            initial_objective = iv[_optVar].max();
        } else if(_method == MAX) {
            initial_objective = iv[_optVar].min();
        }
    }
    /*
    Search::Options o;
    o.stop = Driver::CombinedStop::create(opt.node(), opt.fail(), opt.time(),
                                          true);
    o.c_d = opt.c_d();
    o.a_d = opt.a_d();
    o.threads = opt.threads();
    */
    MySearchOptions o;
    o.stop = Driver::CombinedStop::create(opt.node(), opt.fail(), opt.time(), true);
    o.c_d = opt.c_d();
    o.a_d = opt.a_d();

    o.nb_problems = opt.problems();
    o.mode_decomposition = opt.mode_decomposition();

    o.threads = opt.threads();
    o.threads = o.expand().threads;
    o.cutoff = Gecode::Driver::createCutoff(opt);

    o.mode_search = opt.search();
    o.first_level = opt.firstLevel();

    if(opt.obj_file()) {
        o.obj_file = opt.obj_file();
    }
    _time_subproblems_workers = new std::vector< std::vector<unsigned int> >();

    if (opt.interrupt())
        Driver::CombinedStop::installCtrlHandler(true);
    //Meta<Engine, MyFlatZincSpace> se(this, o); //Meta Problem with SearchOption !!!
    Engine<MyFlatZincSpace> se(this, o); //Meta Problem with SearchOption !!!

    int noOfSolutions = _method == SAT ? opt.solutions() : 0;
    bool printAll = opt.allSolutions();
    int findSol = noOfSolutions;
    MyFlatZincSpace* sol = NULL;

    int objective = -1;
    int nbsolutions = 0;
    unsigned int time_first_solution = 0;
    unsigned int time_last_solution = 0;

    while (MyFlatZincSpace* next_sol = se.next()) {

        nbsolutions++;
        time_last_solution = static_cast<unsigned int>(floor(t_total.stop()));
        if(nbsolutions == 1) {
            time_first_solution = time_last_solution;
        }

        delete sol;
        sol = next_sol;
        if (printAll) {
            sol->print(out, p);
            out << "----------" << std::endl;
        }
        if(_method != SAT) {
            if(_method == MIN) {
                objective = sol->iv[sol->_optVar].max();
            } else {
                objective = sol->iv[sol->_optVar].min();
            }
        }

        if (--findSol==0) {
            delete sol;
            goto stopped;
        }
    }
    if (sol && !printAll) {
        sol->print(out, p);
        out << "----------" << std::endl;
    }
    if (!se.stopped()) {
        if (sol) {
            out << "==========" << endl;
        } else {
            out << "=====UNSATISFIABLE=====" << endl;
        }
    } else if (!sol) {
        out << "=====UNKNOWN=====" << endl;
    }
    delete sol;
stopped:

    if (opt.interrupt())
        Driver::CombinedStop::installCtrlHandler(false);

    string type_instance("ALL_SOLUTIONS");
    if(_method != SAT) {
        type_instance = "OPTIMISATION";
    }

    string status("UNSAT");
    if(_method == SAT && nbsolutions) {
        status = "SAT";
    } else if(_method == MIN && nbsolutions) {
        status = "MINIMIZE";
    } else if(_method == MAX && nbsolutions) {
        status = "MAXIMIZE";
    }

    string mode_decomposition("simple");
    if(opt.mode_decomposition() == MyFlatZincOptions::ModeDecomposition::DBDFS) {
        mode_decomposition = "dbdfs";
    } else if(opt.mode_decomposition() == MyFlatZincOptions::ModeDecomposition::DBDFSwP) {
        mode_decomposition = "dbdfswP";
    }

    string type_search("bab");
    if (opt.search() == MyFlatZincOptions::FZ_SEARCH_EPS) {
        type_search = "eps";
    } else if(_method == SAT) {
        type_search = "dfs";
    } else if (opt.search() == MyFlatZincOptions::FZ_SEARCH_EPS_GRID_GENERATION) {
        type_search = "eps_grid_generation";
    } else if (opt.search() == MyFlatZincOptions::FZ_SEARCH_EPS_GRID_COMPUTATION) {
        type_search = "eps_grid_computation";
    }

    string timesubproblems;
    unsigned int sum_timesubproblems = 0;
    unsigned int min_timesubproblems = 0;
    unsigned int max_timesubproblems = 0;
    bool first_time_affect_min_max = true;

    if(_time_subproblems_workers && _time_subproblems_workers->size()) {
        //cerr << _time_subproblems.size();
        for(size_t i = 0; i < _time_subproblems_workers->size(); i++) {
            for(size_t j = 0; j < (*_time_subproblems_workers)[i].size(); j++) {
                sum_timesubproblems += (*_time_subproblems_workers)[i][j];
                timesubproblems += stl_util::Convert2String((*_time_subproblems_workers)[i][j] / 1000.0) + " ";
            }

            if((*_time_subproblems_workers)[i].size()) {

                unsigned int min_time = *std::min_element((*_time_subproblems_workers)[i].begin(), (*_time_subproblems_workers)[i].end());
                unsigned int max_time = *std::max_element((*_time_subproblems_workers)[i].begin(), (*_time_subproblems_workers)[i].end());

                if(first_time_affect_min_max) {
                    min_timesubproblems = min_time;
                    max_timesubproblems = max_time;
                    first_time_affect_min_max = false;
                } else {
                    if(min_timesubproblems > min_time) {
                        min_timesubproblems = min_time;
                    }

                    if(max_timesubproblems < max_time) {
                        max_timesubproblems = max_time;
                    }
                }
            }
        }
    }
    unsigned int time_total = static_cast<unsigned int>(floor(t_total.stop()));
    unsigned int time_solve = static_cast<unsigned int>(floor(t_solve.stop()));


    if (opt.mode() == SM_STAT) {
        Gecode::Search::Statistics stat = se.statistics();
        out << endl
            << "%%  instance:       "
            << name_instance << endl
            << string(opt.sp_file() ? string("%%  sp_file:       ") + stl_util::Convert2String(opt.sp_file()) + string("\n") : "")
            << string(opt.obj_file() ? string("%%  obj_file:       ") + stl_util::Convert2String(opt.obj_file()) + string("\n") : "")
            << string(opt.add_ub() ? "%%  add ub:       " + stl_util::Convert2String(opt.ub()) + "\n" : "")
            << string(opt.add_lb() ? "%%  add lb:       " + stl_util::Convert2String(opt.lb()) + "\n" : "")
            << "%%  runtime:       "
            //Driver::stop(t_total,out);
            //out << endl
            << time_total / 1000.0 << " (" << time_total << " ms)" << endl
            << "%%  solvetime:     "
            //Driver::stop(t_solve,out);
            //out << endl
            << time_solve / 1000.0 << " (" << time_solve << " ms)" << endl
            << "%%  workers:     "
            << static_cast<int>(opt.threads()) << endl
            << "%%  is search stopped:     "
            << se.stopped() << endl
            << "%%  type search:     "
            << type_search << endl
            << "%%  mode decomposition:     "
            << mode_decomposition << endl
            << "%%  time decomposition:     "
            << this->_time_decomposition / 1000.0 << " (" << this->_time_decomposition << " ms)" << endl
            << "%%  time first solution:     "
            << (nbsolutions > 0 ? time_first_solution / 1000.0 : -1) << " (" << (nbsolutions > 0 ? time_first_solution : -1) << " ms)" << endl
            << "%%  time last solution:     "
            << (nbsolutions > 0 ? time_last_solution / 1000.0 : -1) << " (" << (nbsolutions > 0 ? time_last_solution : -1) << " ms)" << endl
            << "%%  solutions:     "
            << nbsolutions << endl
            << "%%  type instance:     "
            << type_instance << endl
            << "%%  status:     "
            << status << endl
            << "%%  initial_objective:     "
            << initial_objective << endl
            << "%%  objective:     "
            << objective << endl
            << "%%  decision variables:     "
            << (iv.size() + bv.size() + sv.size() + fv.size()) << endl
            << "%%  variables:     "
            << (intVarCount + boolVarCount + setVarCount + floatVarCount) << endl
            << "%%  propagators:   " << n_p << endl
            << "%%  propagations:  " << sstat.propagate+stat.propagate << endl
            << "%%  nodes:         " << stat.node << endl
            << "%%  failures:      " << stat.fail << endl
            << "%%  peak depth:    " << stat.depth << endl
            //<< "%%  peak memory:   "
            //<< static_cast<int>((stat.memory+1023) / 1024) << " KB\n"
            << "%%  depth decomposition:     "
            << this->_depth_decomposition << endl
            << "%%  iterations decomposition:     "
            << this->_iterations_decomposition << endl
            << "%%  expected problems decomposition:     "
            << opt.problems() << endl
            << "%%  generated problems decomposition:     "
            << this->_problems << endl
            << "%%  time max inactivity worker:     "
            << this->_time_max_inactivity / 1000.0 << " (" << this->_time_max_inactivity << " ms)" << endl
            << "%%  nodes decomposition:         " << this->_nodes_decomposition << endl
            << "%%  failures decomposition:      " << this->_fails_decomposition << endl
            //<< "%%  peak memory decomposition:   "
            //<< static_cast<int>((this->_memory_decomposition+1023) / 1024) << " KB\n"
            << "%%  sum time problems:     "
            << sum_timesubproblems / 1000.0 << " (" << sum_timesubproblems << " ms)" << endl
            << "%%  min time problems:     "
            << min_timesubproblems / 1000.0 << " (" << min_timesubproblems << " ms)" << endl
            << "%%  max time problems:     "
            << max_timesubproblems / 1000.0 << " (" << max_timesubproblems << " ms)" << endl
            << "%%  time problems:     "
            << timesubproblems << endl;
    }

    delete _time_subproblems_workers;
    _time_subproblems_workers = NULL;

    delete this->_name_instance;
    this->_name_instance = NULL;
}

void MyFlatZincSpace::createBranchers(Gecode::FlatZinc::AST::Node* ann, int seed, double decay,
                                      bool ignoreUnknown,
                                      std::ostream& err) {
    //std::cerr << "createBranchers\n";
    //if(_space_hook) {
    //    delete _space_hook;
    //}

    //Space stable
    //StatusStatistics sstat;
    //this->status(sstat);

    Rnd rnd(static_cast<unsigned int>(seed));
    TieBreak<IntVarBranch> def_int_varsel = INT_VAR_AFC_SIZE_MAX(0.99);
    IntValBranch def_int_valsel = INT_VAL_MIN();
    TieBreak<IntVarBranch> def_bool_varsel = INT_VAR_AFC_MAX(0.99);
    IntValBranch def_bool_valsel = INT_VAL_MIN();
#ifdef GECODE_HAS_SET_VARS
    SetVarBranch def_set_varsel = SET_VAR_AFC_SIZE_MAX(0.99);
    SetValBranch def_set_valsel = SET_VAL_MIN_INC();
#endif
#ifdef GECODE_HAS_FLOAT_VARS
    TieBreak<FloatVarBranch> def_float_varsel = FLOAT_VAR_SIZE_MIN();
    FloatValBranch def_float_valsel = FLOAT_VAL_SPLIT_MIN();
#endif

    std::vector<bool> iv_searched(iv.size());
    for (unsigned int i=iv.size(); i--;)
        iv_searched[i] = false;
    std::vector<bool> bv_searched(bv.size());
    for (unsigned int i=bv.size(); i--;)
        bv_searched[i] = false;
#ifdef GECODE_HAS_SET_VARS
    std::vector<bool> sv_searched(sv.size());
    for (unsigned int i=sv.size(); i--;)
        sv_searched[i] = false;
#endif
#ifdef GECODE_HAS_FLOAT_VARS
    std::vector<bool> fv_searched(fv.size());
    for (unsigned int i=fv.size(); i--;)
        fv_searched[i] = false;
#endif

    if (ann) {
        std::vector<AST::Node*> flatAnn;
        if (ann->isArray()) {
            flattenAnnotations(ann->getArray()  , flatAnn);
        } else {
            flatAnn.push_back(ann);
        }

        for (unsigned int i=0; i<flatAnn.size(); i++) {
            if (flatAnn[i]->isCall("gecode_search")) {
                //AST::Call* c = flatAnn[i]->getCall();
                //branchWithPlugin(c->args);
            } else if (flatAnn[i]->isCall("int_search")) {
                AST::Call *call = flatAnn[i]->getCall("int_search");
                AST::Array *args = call->getArgs(4);
                AST::Array *vars = args->a[0]->getArray();
                int k=vars->a.size();
                for (int i=vars->a.size(); i--;)
                    if (vars->a[i]->isInt())
                        k--;
                IntVarArgs va(k);
                vector<string> names;
                k=0;
                for (unsigned int i=0; i<vars->a.size(); i++) {
                    if (vars->a[i]->isInt())
                        continue;
                    va[k++] = iv[vars->a[i]->getIntVar()];
                    iv_searched[vars->a[i]->getIntVar()] = true;
                    names.push_back(vars->a[i]->getVarName());
                }
                std::string r0, r1;
                BrancherHandle bh = branch(*this, va,
                                           ann2ivarsel(args->a[1],rnd,decay),
                                           ann2ivalsel(args->a[2],r0,r1,rnd),
                                           NULL,
                                           &varValPrint<IntVar>);
                branchInfo.add(bh,r0,r1,names);
            } else if (flatAnn[i]->isCall("int_assign")) {
                AST::Call *call = flatAnn[i]->getCall("int_assign");
                AST::Array *args = call->getArgs(2);
                AST::Array *vars = args->a[0]->getArray();
                int k=vars->a.size();
                for (int i=vars->a.size(); i--;)
                    if (vars->a[i]->isInt())
                        k--;
                IntVarArgs va(k);
                k=0;
                for (unsigned int i=0; i<vars->a.size(); i++) {
                    if (vars->a[i]->isInt())
                        continue;
                    va[k++] = iv[vars->a[i]->getIntVar()];
                    iv_searched[vars->a[i]->getIntVar()] = true;
                }
                assign(*this, va, ann2asnivalsel(args->a[1],rnd), NULL,
                       &varValPrint<IntVar>);
            } else if (flatAnn[i]->isCall("bool_search")) {
                AST::Call *call = flatAnn[i]->getCall("bool_search");
                AST::Array *args = call->getArgs(4);
                AST::Array *vars = args->a[0]->getArray();
                int k=vars->a.size();
                for (int i=vars->a.size(); i--;)
                    if (vars->a[i]->isBool())
                        k--;
                BoolVarArgs va(k);
                k=0;
                vector<string> names;
                for (unsigned int i=0; i<vars->a.size(); i++) {
                    if (vars->a[i]->isBool())
                        continue;
                    va[k++] = bv[vars->a[i]->getBoolVar()];
                    bv_searched[vars->a[i]->getBoolVar()] = true;
                    names.push_back(vars->a[i]->getVarName());
                }

                std::string r0, r1;
                BrancherHandle bh = branch(*this, va,
                                           ann2ivarsel(args->a[1],rnd,decay),
                                           ann2ivalsel(args->a[2],r0,r1,rnd), NULL,
                                           &varValPrint<BoolVar>);
                branchInfo.add(bh,r0,r1,names);
            } else if (flatAnn[i]->isCall("int_default_search")) {
                AST::Call *call = flatAnn[i]->getCall("int_default_search");
                AST::Array *args = call->getArgs(2);
                def_int_varsel = ann2ivarsel(args->a[0],rnd,decay);
                std::string r0;
                def_int_valsel = ann2ivalsel(args->a[1],r0,r0,rnd);
            } else if (flatAnn[i]->isCall("bool_default_search")) {
                AST::Call *call = flatAnn[i]->getCall("bool_default_search");
                AST::Array *args = call->getArgs(2);
                def_bool_varsel = ann2ivarsel(args->a[0],rnd,decay);
                std::string r0;
                def_bool_valsel = ann2ivalsel(args->a[1],r0,r0,rnd);
            } else if (flatAnn[i]->isCall("set_search")) {
#ifdef GECODE_HAS_SET_VARS
                AST::Call *call = flatAnn[i]->getCall("set_search");
                AST::Array *args = call->getArgs(4);
                AST::Array *vars = args->a[0]->getArray();
                int k=vars->a.size();
                for (int i=vars->a.size(); i--;)
                    if (vars->a[i]->isSet())
                        k--;
                SetVarArgs va(k);
                k=0;
                vector<string> names;
                for (unsigned int i=0; i<vars->a.size(); i++) {
                    if (vars->a[i]->isSet())
                        continue;
                    va[k++] = sv[vars->a[i]->getSetVar()];
                    sv_searched[vars->a[i]->getSetVar()] = true;
                    names.push_back(vars->a[i]->getVarName());
                }
                std::string r0, r1;
                BrancherHandle bh = branch(*this, va,
                                           ann2svarsel(args->a[1],rnd,decay),
                                           ann2svalsel(args->a[2],r0,r1,rnd),
                                           NULL,
                                           &varValPrint<SetVar>);
                branchInfo.add(bh,r0,r1,names);
#else
                if (!ignoreUnknown) {
                    err << "Warning, ignored search annotation: ";
                    flatAnn[i]->print(err);
                    err << std::endl;
                }
#endif
            } else if (flatAnn[i]->isCall("set_default_search")) {
#ifdef GECODE_HAS_SET_VARS
                AST::Call *call = flatAnn[i]->getCall("set_default_search");
                AST::Array *args = call->getArgs(2);
                def_set_varsel = ann2svarsel(args->a[0],rnd,decay);
                std::string r0;
                def_set_valsel = ann2svalsel(args->a[1],r0,r0,rnd);
#else
                if (!ignoreUnknown) {
                    err << "Warning, ignored search annotation: ";
                    flatAnn[i]->print(err);
                    err << std::endl;
                }
#endif
            } else if (flatAnn[i]->isCall("float_default_search")) {
#ifdef GECODE_HAS_FLOAT_VARS
                AST::Call *call = flatAnn[i]->getCall("float_default_search");
                AST::Array *args = call->getArgs(2);
                def_float_varsel = ann2fvarsel(args->a[0],rnd,decay);
                std::string r0;
                def_float_valsel = ann2fvalsel(args->a[1],r0,r0);
#else
                if (!ignoreUnknown) {
                    err << "Warning, ignored search annotation: ";
                    flatAnn[i]->print(err);
                    err << std::endl;
                }
#endif
            } else if (flatAnn[i]->isCall("float_search")) {
#ifdef GECODE_HAS_FLOAT_VARS
                AST::Call *call = flatAnn[i]->getCall("float_search");
                AST::Array *args = call->getArgs(5);
                AST::Array *vars = args->a[0]->getArray();
                int k=vars->a.size();
                for (int i=vars->a.size(); i--;)
                    if (vars->a[i]->isFloat())
                        k--;
                FloatVarArgs va(k);
                k=0;
                vector<string> names;
                for (unsigned int i=0; i<vars->a.size(); i++) {
                    if (vars->a[i]->isFloat())
                        continue;
                    va[k++] = fv[vars->a[i]->getFloatVar()];
                    fv_searched[vars->a[i]->getFloatVar()] = true;
                    names.push_back(vars->a[i]->getVarName());
                }
                std::string r0, r1;
                BrancherHandle bh = branch(*this, va,
                                           ann2fvarsel(args->a[2],rnd,decay),
                                           ann2fvalsel(args->a[3],r0,r1),
                                           NULL,
                                           &varValPrintF);
                branchInfo.add(bh,r0,r1,names);
#else
                if (!ignoreUnknown) {
                    err << "Warning, ignored search annotation: ";
                    flatAnn[i]->print(err);
                    err << std::endl;
                }
#endif
            } else {
                if (!ignoreUnknown) {
                    err << "Warning, ignored search annotation: ";
                    flatAnn[i]->print(err);
                    err << std::endl;
                }
            }
        }
    }
    int introduced = 0;
    int funcdep = 0;
    int searched = 0;
    for (int i=iv.size(); i--;) {
        if (iv_searched[i]) {
            searched++;
        } else if (iv_introduced[2*i]) {
            if (iv_introduced[2*i+1]) {
                funcdep++;
            } else {
                introduced++;
            }
        }
    }
    IntVarArgs iv_sol(iv.size()-(introduced+funcdep+searched));
    IntVarArgs iv_tmp(introduced);
    for (int i=iv.size(), j=0, k=0; i--;) {
        if (iv_searched[i])
            continue;
        if (iv_introduced[2*i]) {
            if (!iv_introduced[2*i+1]) {
                iv_tmp[j++] = iv[i];
            }
        } else {
            iv_sol[k++] = iv[i];
        }
    }

    introduced = 0;
    funcdep = 0;
    searched = 0;
    for (int i=bv.size(); i--;) {
        if (bv_searched[i]) {
            searched++;
        } else if (bv_introduced[2*i]) {
            if (bv_introduced[2*i+1]) {
                funcdep++;
            } else {
                introduced++;
            }
        }
    }
    BoolVarArgs bv_sol(bv.size()-(introduced+funcdep+searched));
    BoolVarArgs bv_tmp(introduced);
    for (int i=bv.size(), j=0, k=0; i--;) {
        if (bv_searched[i])
            continue;
        if (bv_introduced[2*i]) {
            if (!bv_introduced[2*i+1]) {
                bv_tmp[j++] = bv[i];
            }
        } else {
            bv_sol[k++] = bv[i];
        }
    }

    if (iv_sol.size() > 0)
        branch(*this, iv_sol, def_int_varsel, def_int_valsel);
    if (bv_sol.size() > 0)
        branch(*this, bv_sol, def_bool_varsel, def_bool_valsel);
#ifdef GECODE_HAS_FLOAT_VARS
    introduced = 0;
    funcdep = 0;
    searched = 0;
    for (int i=fv.size(); i--;) {
        if (fv_searched[i]) {
            searched++;
        } else if (fv_introduced[2*i]) {
            if (fv_introduced[2*i+1]) {
                funcdep++;
            } else {
                introduced++;
            }
        }
    }
    FloatVarArgs fv_sol(fv.size()-(introduced+funcdep+searched));
    FloatVarArgs fv_tmp(introduced);
    for (int i=fv.size(), j=0, k=0; i--;) {
        if (fv_searched[i])
            continue;
        if (fv_introduced[2*i]) {
            if (!fv_introduced[2*i+1]) {
                fv_tmp[j++] = fv[i];
            }
        } else {
            fv_sol[k++] = fv[i];
        }
    }

    if (fv_sol.size() > 0)
        branch(*this, fv_sol, def_float_varsel, def_float_valsel);
#endif
#ifdef GECODE_HAS_SET_VARS
    introduced = 0;
    funcdep = 0;
    searched = 0;
    for (int i=sv.size(); i--;) {
        if (sv_searched[i]) {
            searched++;
        } else if (sv_introduced[2*i]) {
            if (sv_introduced[2*i+1]) {
                funcdep++;
            } else {
                introduced++;
            }
        }
    }
    SetVarArgs sv_sol(sv.size()-(introduced+funcdep+searched));
    SetVarArgs sv_tmp(introduced);
    for (int i=sv.size(), j=0, k=0; i--;) {
        if (sv_searched[i])
            continue;
        if (sv_introduced[2*i]) {
            if (!sv_introduced[2*i+1]) {
                sv_tmp[j++] = sv[i];
            }
        } else {
            sv_sol[k++] = sv[i];
        }
    }

    if (sv_sol.size() > 0)
        branch(*this, sv_sol, def_set_varsel, def_set_valsel);
#endif
    iv_aux = IntVarArray(*this, iv_tmp);
    bv_aux = BoolVarArray(*this, bv_tmp);
    int n_aux = iv_aux.size() + bv_aux.size();
#ifdef GECODE_HAS_SET_VARS
    sv_aux = SetVarArray(*this, sv_tmp);
    n_aux =+ sv_aux.size();
#endif
#ifdef GECODE_HAS_FLOAT_VARS
    fv_aux = FloatVarArray(*this, fv_tmp);
    n_aux =+ fv_aux.size();
#endif
    if (n_aux > 0) {
        AuxVarBrancher::post(*this, def_int_varsel, def_int_valsel,
                             def_bool_varsel, def_bool_valsel
#ifdef GECODE_HAS_SET_VARS
                             , def_set_varsel, def_set_valsel
#endif
#ifdef GECODE_HAS_FLOAT_VARS
                             , def_float_varsel, def_float_valsel
#endif
                            );
    }
}

Gecode::Space* MyFlatZincSpace::copy(bool share) {
    //Gecode::StatusStatistics sstat;
    //this->status(sstat);
    return new MyFlatZincSpace(share, *this);
}

MyFlatZincSpace::MyFlatZincSpace(bool share, MyFlatZincSpace& f)
    : Gecode::FlatZinc::FlatZincSpace(share, f), _space_hook(NULL),
      _time_decomposition(f._time_decomposition),
      _problems(f._problems), _depth_decomposition(f._depth_decomposition),
      _iterations_decomposition(f._iterations_decomposition),
      _nodes_decomposition(f._nodes_decomposition),
      _fails_decomposition(f._fails_decomposition),
      _memory_decomposition(f._memory_decomposition),
      _time_max_inactivity(f._time_max_inactivity),
      _time_subproblems_workers(NULL),
      _name_instance(f._name_instance)

  /*,
  branch_var_iv(f.branch_var_iv),
  branch_val_iv(f.branch_val_iv),
  args_iv(f.args_iv),

  branch_var_bv(f.branch_var_bv),
  branch_val_bv(f.branch_val_bv),
  args_bv(f.args_bv),

  #ifdef GECODE_HAS_SET_VARS
  branch_var_sv(f.branch_var_sv),
  branch_val_sv(f.branch_val_sv),
  args_sv(f.args_sv),
  #endif

  #ifdef GECODE_HAS_FLOAT_VARS
  branch_var_fv(f.branch_var_fv),
  branch_val_fv(f.branch_val_fv),
  args_fv(f.args_fv)
  #endif
  */
{
    /*
    iv = f.iv;
    for(int i = 0; i < f.iv.size(); i++) {
        iv[i].update(*this, share, f.iv[i]);
    }
    */
    /*
    bv = f.bv;
    for(int i = 0; i < f.bv.size(); i++) {
        bv[i].update(*this, share, f.bv[i]);
    }


    #ifdef GECODE_HAS_SET_VARS

    sv_decision = f.sv_decision;
    for(int i = 0; i < f.sv_decision.size(); i++) {
        sv_decision[i].update(*this, share, f.sv_decision[i]);
    }


    #endif
    #ifdef GECODE_HAS_FLOAT_VARS

    fv_decision = f.fv_decision;
    for(int i = 0; i < f.fv_decision.size(); i++) {
        fv_decision[i].update(*this, share, f.fv_decision[i]);
    }

    #endif
    */
    /*
    iv_introduced = f.iv_introduced;
    bv_introduced = f.bv_introduced;
    fv_introduced = f.fv_introduced;
    sv_introduced = f.sv_introduced;
    */
}


/// Comparison class for sorting using \a <
template<class Var>
struct VarCompare {
    bool _less;

    VarCompare(bool less) : _less(less) {}

    bool operator ()(const Var& lhs, const Var& rhs) {
        return _less ? lhs.size() < rhs.size() : lhs.size() > rhs.size();
    }
};

template<class ArrayVar>
struct ArrayVarIndexCompare {
    bool _less;
    const ArrayVar& base_arr;

    ArrayVarIndexCompare (const ArrayVar& arr, bool less) : base_arr(arr), _less(less) {}

    bool operator () (int a, int b) const {
        return _less ? base_arr[a].size() < base_arr[b].size() : base_arr[a].size() > base_arr[b].size();
    }
};

void MyFlatZincSpace::sortVariables(bool less) {

    /*
    std::cerr << "int vars : ";
    for(int i = 0; i < iv.size(); i++) {
        std::cerr << "[" << iv[i].min() << "..." << iv[i].max() << "], ";
    }
    std::cerr << std::endl;

    std::cerr << "bool vars : ";
    for(int i = 0; i < bv.size(); i++) {
        std::cerr << "[" << bv[i].min() << "..." << bv[i].max() << "], ";
    }
    */
    if(iv.size()) {
        int size_iv = iv.size();
        if(_optVar != -1) {
            //Update index _optVar when sort variables

            std::vector<int> vec_indexes(size_iv);
            for(int i = 0; i < size_iv; i++) {
                vec_indexes[i] = i;
            }

            std::stable_sort(vec_indexes.begin(),
                             vec_indexes.end(),
                             ArrayVarIndexCompare< Gecode::IntVarArray>(iv, less));

            //std::cerr << _optVar << endl;
            for(int i = 0; i < size_iv; i++) {
                if(vec_indexes[i] == _optVar) {
                    _optVar = i;
                    break;
                }
            }
        }
        IntVarArgs iv_args(iv);
        /*
        for(int i = 0; i < size_iv; i++) {
            iv_args[i] = iv[i];
        }
        */
        std::stable_sort(iv_args.begin(), iv_args.end(), VarCompare<Gecode::IntVar>(less));
        iv = IntVarArray(*this, iv_args);
        //Faire dans le shrinkArray
    }

    if(bv.size()) {
        BoolVarArgs bv_args(bv);
        std::stable_sort(bv.begin(), bv.end(), VarCompare<Gecode::BoolVar>(less));
        bv = BoolVarArray(*this, bv_args);
    }
    /*
    std::cerr << "int vars : ";
    for(int i = 0; i < iv.size(); i++) {
        std::cerr << "[" << iv[i].min() << "..." << iv[i].max() << "], ";
    }
    std::cerr << std::endl;

    std::cerr << "bool vars : ";
    for(int i = 0; i < bv.size(); i++) {
        std::cerr << "[" << bv[i].min() << "..." << bv[i].max() << "], ";
    }
    std::cerr << std::endl;
    */

#ifdef GECODE_HAS_SET_VARS

    //std::stable_sort(sv_decision.begin(), sv_decision.end(), VarCompare<Gecode::SetVar>(less));

#endif
#ifdef GECODE_HAS_FLOAT_VARS

    if(fv.size()) {
        std::stable_sort(fv.begin(), fv.end(), VarCompare<Gecode::FloatVar>(less));
    }
#endif

}

//Constraint Upperbound
void MyFlatZincSpace::constrain(const Space& s) {
    if (_optVarIsInt) {
        if (_method == MIN)
            rel(*this, iv[_optVar], Gecode::IRT_LE,
                static_cast<const MyFlatZincSpace*>(&s)->iv[_optVar].max());
        else if (_method == MAX)
            rel(*this, iv[_optVar], Gecode::IRT_GR,
                static_cast<const MyFlatZincSpace*>(&s)->iv[_optVar].min());
    } else {
#ifdef GECODE_HAS_FLOAT_VARS
        if (_method == MIN)
            rel(*this, fv[_optVar], FRT_LE,
                static_cast<const FlatZincSpace*>(&s)->fv[_optVar].max());
        else if (_method == MAX)
            rel(*this, fv[_optVar], FRT_GR,
                static_cast<const FlatZincSpace*>(&s)->fv[_optVar].min());
#endif
    }
}


//Problem in windows with linker (ArgArrayBase<T>::operator delete no in lib)
/*
#ifdef _MSC_VER
template<class T>
void Gecode::ArgArrayBase<T>::operator delete(void * a,unsigned int size_t) {
}
#endif
*/