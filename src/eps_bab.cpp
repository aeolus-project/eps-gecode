/*---------------------------------------------------------------------------*/
/*                                                                           */
/* eps_bab.cpp													                                      */
/*                                                                           */
/* Author : Mohamed REZGUI (m.rezgui06@gmail.com)                              */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/* Copyright (c) 2014 Mohamed REZGUI. All rights reserved.
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

#include <gecode/int/branch.hh>
#include <gecode/search/parallel/engine.hh>
#include <gecode/int.hh>
#include <gecode/search/sequential/dfs.hh>
#include <gecode/search/sequential/bab.hh>

#include <vector>
#include <string>
#include <list>
#include <fstream>

#include "search.h"
#include "flatzinc.h"
#include "stl_util.h"
#include "lock.h"

using namespace stl_util;

struct ProblemCompare {
    bool _less;

    ProblemCompare(bool less) : _less(less) {}

    bool operator ()(MyFlatZincSpace* lhs, MyFlatZincSpace* rhs) {
        return lhs->method() == MyFlatZincSpace::MIN ?
               lhs->iv[lhs->optVar()].max() < rhs->iv[rhs->optVar()].max()
               : lhs->iv[lhs->optVar()].min() > rhs->iv[rhs->optVar()].min();
    }
};


/// Implementation of depth-first branch-and-bound search engine with limited depth
class BoundedBAB : public Gecode::Search::Worker {
private:
    /// Search options
    Gecode::Search::Options opt;
    /// Distance until next clone
    unsigned int d;

public:
    /// Current path in search tree
    Gecode::Search::Sequential::Path path;
    /// Number of entries not yet constrained to be better
    int mark;
    /// Current space being explored
    Gecode::Space* cur;
    /// Best solution found so far
    Gecode::Space* best;

    /// Initialize with space \a s (of size \a sz) and search options \a o
    BoundedBAB(Gecode::Space* s, const Gecode::Search::Options& o);
    /// %Search for next better solution
    Gecode::Space* next(void);
    /// Return statistics
    Gecode::Search::Statistics statistics(void) const;
    /// Reset engine to restart at space \a s and return new root
    void reset(Gecode::Space* s);
    /// Return no-goods
    Gecode::NoGoods& nogoods(void);
    /// Destructor
    ~BoundedBAB(void);
};

/// Implementation of depth-first branch-and-bound search engine with limited depth
forceinline
BoundedBAB::BoundedBAB(Gecode::Space* s, const Gecode::Search::Options& o)
    : Worker(), opt(o), path(static_cast<int>(opt.nogoods_limit)),
      d(0), mark(0), best(NULL) {
    if ((s == NULL) || (s->status(*this) == Gecode::SS_FAILED)) {
        fail++;
        cur = NULL;
        if (!o.clone)
            delete s;
    } else {
        cur = snapshot(s,opt);
    }
}
static double min_size_problem = 0.0;

forceinline Gecode::Space*
BoundedBAB::next(void) {
    /*
     * The invariant maintained by the engine is:
     *   For all nodes stored at a depth less than mark, there
     *   is no guarantee of betterness. For those above the mark,
     *   betterness is guaranteed.
     *
     * The engine maintains the path on the stack for the current
     * node to be explored.
     *
     */
    start();
    while (true) {
        while (cur) {
            if (stop(opt))
                return NULL;
            node++;
            switch (cur->status(*this)) {
            case Gecode::SS_FAILED:
                fail++;
                delete cur;
                cur = NULL;
                break;
            case Gecode::SS_SOLVED:
                // Deletes all pending branchers
                (void) cur->choice();
                delete best;
                best = cur;
                cur = NULL;
                mark = path.entries();

                //Remove problem memory leak;
                //return best->clone();
                return best;
            case Gecode::SS_BRANCH: {

                //std::cerr << "Good Problem\n";

                Gecode::Space* c;
                if ((d == 0) || (d >= opt.c_d)) {
                    c = cur->clone();
                    d = 1;
                } else {
                    c = NULL;
                    d++;
                }

                const Gecode::Choice* ch = path.push(*this,cur,c);
                cur->commit(*ch,0);

                break;
            }
            default:
                GECODE_NEVER;
            }
        }
        // Recompute and add constraint if necessary
        do {
            if (!path.next())
                return NULL;
            cur = path.recompute(d,opt.a_d,*this,best,mark);
        } while (cur == NULL);
    }
    GECODE_NEVER;
    return NULL;
}

forceinline Gecode::Search::Statistics
BoundedBAB::statistics(void) const {
    return *this;
}

forceinline void
BoundedBAB::reset(Gecode::Space* s) {
    delete best;
    best = NULL;
    path.reset();
    d = mark = 0;
    delete cur;

    if (s->status(*this) == Gecode::SS_FAILED) {
        cur = NULL;
    } else {
        cur = s;
    }
    Worker::reset();

}

forceinline Gecode::NoGoods&
BoundedBAB::nogoods(void) {
    return path;
}
forceinline
BoundedBAB::~BoundedBAB(void) {
    path.reset();
    delete best;
    delete cur;
}


namespace Parallel {


/// %Parallel branch-and-bound engine
class EPS_BAB : public Gecode::Search::Parallel::Engine {
protected:
    /// %Parallel branch-and-bound search worker
    class Worker : public Gecode::Search::Parallel::Engine::Worker {
    protected:
        /// Number of entries not yet constrained to be better
        int mark;
    public:
        /// Best solution found so far
        Gecode::Space* best;
        enum ModeSearch {
            DECOMPOSITION,    //< DECOMPOSITION
            RESOLUTION        //< RESOLUTION
        } mode_search;

        ///control if the worker is done
        bool done;
        unsigned int id;

        Gecode::TupleSet* _tuples_int_ndi;
        Gecode::TupleSet* _tuples_bool_ndi;
        std::vector<int> _group_tuples;

        Gecode::Support::Timer _timer_problem;

        /// decomposeProblems
        void decomposeProblems(MyFlatZincSpace* s, const MySearchOptions& o);

        /// Initialize for space \a s (of size \a sz) with engine \a e
        Worker(Gecode::Space* s, EPS_BAB& e, unsigned int id_worker);
        /// Provide access to engine
        EPS_BAB& engine(void) const;
        /// Start execution of worker
        virtual void run(void);
        /// Accept better solution \a b
        void better(Gecode::Space* b);
        /// Try to find some work
        void find(void);
        void reset(Gecode::Space* s);
        /// Destructor
        virtual ~Worker(void);

    };
    /// Array of worker references
    Worker** _workers;
    /// Best solution so far
    Gecode::Space* best;

    Worker* _master; //for sequential decomposition

    enum ModeDecomposition {
        SEQUENTIAL,    //< SEQUENTIAL
        PARALLEL       //< PARALLEL
    } _mode_decomposition;

    /// space generated by parsing flatzinc file
    MyFlatZincSpace* _space_home;

    MySearchOptions optSearch;


    int _current_problem_decomposition;
    int _current_index_tuple_decomposition;
    int _current_index_problem_decomposition;

    int _current_problem_resolution;
    int _current_index_tuple_resolution;
    int _current_index_problem_resolution;

    int _current_problem;

    int _current_index_group_tuple_resolution;

    std::vector<int> _groups_tuples_resolution;
    std::vector<Gecode::TupleSet*> _tuples_bool_resolution;
    std::vector<Gecode::TupleSet*> _tuples_int_resolution;

    std::vector<int> _problems_for_decomposition;
    int _nb_workers_decomposition_done;

    //Timer to compute the max_inactivity
    bool _already_timer_max_inactivity_started;
    Gecode::Support::Timer timer_max_inactivity;

    //Timer to compute the duration of decomposition
    Gecode::Support::Timer _timer_decomposition;

    unsigned int getBusyWorkers() {
        return this->n_busy;
    }

    Gecode::Support::Mutex m_findjob_resolution;
    void lockFindJobResolution() {
        m_findjob_resolution.acquire();
    }

    void unlockFindJobResolution() {
        m_findjob_resolution.release();
    }

    Gecode::Support::Mutex m_findjob_decomposition;
    void lockFindJobDecomposition() {
        m_findjob_decomposition.acquire();
    }

    void unlockFindJobDecomposition() {
        m_findjob_decomposition.release();
    }

    void notifyFinishedSubproblem(unsigned int id_worker, double time_problem) {
        (*_space_home->_time_subproblems_workers)[id_worker].push_back(static_cast<unsigned int>(floor(time_problem)));
    }

public:
    /// Provide access to worker \a i
    Worker* worker(unsigned int i) const;

    /// \name Search control
    //@{
    /// Report solution \a s
    void solution(Worker* w);
    //@}
    /// Reset engine to restart at space \a s and return new root
    //TODO See Engine code
    //
    virtual void reset(Gecode::Space* s) {
        // Grab wait lock for reset
        m_wait_reset.acquire();
        // Release workers for reset
        release(C_RESET);
        // Wait for reset cycle started
        e_reset_ack_start.wait();
        // All workers are marked as busy again
        delete best;
        best = NULL;
        n_busy = workers();
        for (unsigned int i=0; i<workers(); i++)
            worker(i)->reset(NULL);
        // Block workers again to ensure invariant
        block();
        // Release reset lock
        m_wait_reset.release();
        // Wait for reset cycle stopped
        e_reset_ack_stop.wait();

    }

    /// \name Engine interface
    //@{
    /// Initialize for space \a s (of size \a sz) with options \a o
    EPS_BAB(Gecode::Space* s, const MySearchOptions& o);
    /// Return statistics
    virtual Gecode::Search::Statistics statistics(void) const;
    /// Return reference to deepest space on the stack
    const Gecode::Space& deepest(void) const;

    /// NotifyBestSolution
    int notifyBestSolution(int valueObj, Gecode::FlatZinc::FlatZincSpace::Meth method);


    Gecode::NoGoods& nogoods(void);
    /// Destructor
    virtual ~EPS_BAB(void);
    //@}
};


/*
 * Engine: basic access routines
 */
forceinline EPS_BAB&
EPS_BAB::Worker::engine(void) const {
    return static_cast<EPS_BAB&>(_engine);
}
forceinline EPS_BAB::Worker*
EPS_BAB::worker(unsigned int i) const {
    if(_workers) {
        return _workers[i];
    }
    return NULL;
}

forceinline const Gecode::Space&
EPS_BAB::deepest(void) const {
    return *_space_home;
}

/*
 * Engine: initialization
 */
forceinline
EPS_BAB::Worker::Worker(Gecode::Space* s, EPS_BAB& e, unsigned int id_worker)
    : Gecode::Search::Parallel::Engine::Worker(s,e),
      mark(0),
      best(NULL),
      done(false),
      mode_search(DECOMPOSITION),
      id(id_worker),
      _tuples_bool_ndi(NULL),
      _tuples_int_ndi(NULL) {
    idle = true;
}

forceinline
EPS_BAB::EPS_BAB(Gecode::Space* s, const MySearchOptions& o)
    : Gecode::Search::Parallel::Engine(o), best(NULL),
      _space_home(static_cast<MyFlatZincSpace*>(s)),
      _already_timer_max_inactivity_started(false),
      _current_problem_decomposition(0),
      _current_index_tuple_decomposition(0),
      _current_index_problem_decomposition(0),
      _current_problem_resolution(0),
      _current_problem(0),
      _current_index_tuple_resolution(0),
      _current_index_problem_resolution(0),
      _current_index_group_tuple_resolution(0),
      _nb_workers_decomposition_done(0),
      _mode_decomposition(PARALLEL), optSearch(o) {

    _workers = NULL;
    _master = new Worker(NULL,*this, -1);

    //Start Timer
    _timer_decomposition.start();

    if(optSearch.mode_decomposition == MyFlatZincOptions::ModeDecomposition::DBDFSwP
            && optSearch.threads > 1) {
        _mode_decomposition = PARALLEL;
    } else {
        _mode_decomposition = SEQUENTIAL;
    }

    //Force sequential
    //_mode_decomposition = SEQUENTIAL;

    if(_mode_decomposition == SEQUENTIAL) {
        optSearch.nb_problems = o.nb_problems;
    } else {
        optSearch.nb_problems = o.threads;
    }

    _space_home->_space_hook->_nodes_decomposition = 0;
    _space_home->_space_hook->_fails_decomposition = 0;
    _space_home->_space_hook->_memory_decomposition = 0;
    _space_home->_space_hook->_iterations_decomposition = 0;
    _space_home->_space_hook->_depth_decomposition = 0;

    _master->done = false;
    //Gecode::Support::Timer t_solve;
    //t_solve.start();

    _master->decomposeProblems(_space_home->_space_hook, optSearch);

    unsigned int time_solve = static_cast<unsigned int>(floor(this->_timer_decomposition.stop()));
    //std::cerr << "Time resolution : " << time_solve << std::endl;
    //getchar();
    //exit(0);

    _master->done = true;

    //exit(0);

    if(_mode_decomposition == SEQUENTIAL) {
        _space_home->_time_decomposition = static_cast<unsigned int>(floor(this->_timer_decomposition.stop()));

        //std::cerr << "decomposition problem done" << std::endl;

        //No worker for decomposition
        _nb_workers_decomposition_done = workers();

        if(!_master->_group_tuples.empty()) {
            _groups_tuples_resolution.insert(_groups_tuples_resolution.end(), _master->_group_tuples.begin(), _master->_group_tuples.end());
            _tuples_bool_resolution.push_back(new Gecode::TupleSet(*_master->_tuples_bool_ndi));
            _tuples_int_resolution.push_back(new Gecode::TupleSet(*_master->_tuples_int_ndi));

            if(!_tuples_bool_resolution.back()->finalized()) {
                _tuples_bool_resolution.back()->finalize();
            }

            if(!_tuples_int_resolution.back()->finalized()) {
                _tuples_int_resolution.back()->finalize();
            }
        }

        delete _master->_tuples_bool_ndi;
        delete _master->_tuples_int_ndi;
        _master->_tuples_bool_ndi = NULL;
        _master->_tuples_int_ndi = NULL;

    } else {

        //Reset the number of workers to do decomposition
        _nb_workers_decomposition_done = 0;

        int nb_problems_workers = o.nb_problems / workers();
        if(o.nb_problems % workers() != 0 || o.nb_problems / workers() == 0) {
            nb_problems_workers++;
        }

        this->_problems_for_decomposition.resize(workers(), nb_problems_workers);
    }

    _space_home->_nodes_decomposition = _space_home->_space_hook->_nodes_decomposition;
    _space_home->_fails_decomposition = _space_home->_space_hook->_fails_decomposition;
    _space_home->_memory_decomposition = _space_home->_space_hook->_memory_decomposition;
    _space_home->_iterations_decomposition = _space_home->_space_hook->_iterations_decomposition;
    _space_home->_depth_decomposition = _space_home->_space_hook->_depth_decomposition;


    _space_home->_problems = _groups_tuples_resolution.size();

    _space_home->_time_max_inactivity = 0;
    _already_timer_max_inactivity_started = false;

    //in case of _mode_decomposition
    if(_master->_group_tuples.empty() && _space_home->_problems == 0) {
        n_busy = 0;
        std::cerr << "Problem resolved in sequential dbdfs decomposition !!!\n";
        return;
    }

    /*
    Gecode::StatusStatistics sstat;
    _space_home->status(sstat);
    _space_home->_space_hook->status(sstat);
    */
    // Create workers
    _workers = static_cast<Worker**>
               (Gecode::heap.ralloc(workers() * sizeof(Worker*)));

    // All other workers start with no work and get the entire search tree
    for (unsigned int i=0; i<workers(); i++) {
        _workers[i] = new Worker(NULL,*this, i); //NULL permit the workers to find a space
        _workers[i]->done = false;
        /*
        if(best) {
            _workers[i]->best = best->clone(false);
        }
        */
        if(_mode_decomposition == SEQUENTIAL) {
            _workers[i]->mode_search = Worker::RESOLUTION;
        } else {
            _workers[i]->mode_search = Worker::DECOMPOSITION;
        }
    }

    //Resize time_subproblems resolved by workers
    _space_home->_time_subproblems_workers->clear();
    _space_home->_time_subproblems_workers->resize(workers());

    // Block all workers
    block();
    // Create and start threads
    for (unsigned int i=0; i<workers(); i++)
        Gecode::Support::Thread::run(_workers[i]);
}

forceinline void
EPS_BAB::Worker::reset(Gecode::Space* s) {
    delete cur;
    delete best;
    best = NULL;
    path.reset(0);
    d = mark = 0;
    idle = false;
    if ((s == NULL) || (s->status(*this) == Gecode::SS_FAILED)) {
        delete s;
    } else {
        cur = s;
    }
    Gecode::Search::Worker::reset();
}


/*
 * Engine: search control
 */
forceinline void
EPS_BAB::Worker::better(Gecode::Space* b) {
    //m.acquire();
    if(b) {
        if(best) {
            delete best;
        }
        best = b->clone(false);
        mark = path.entries();
        /*
        if (cur) {
            cur->constrain(*best);
        }
        */
    }
    //m.release();
}

forceinline int
EPS_BAB::notifyBestSolution(int valueObj, Gecode::FlatZinc::FlatZincSpace::Meth method) {
    //Read File Objective !!!
    if(optSearch.mode_search == MyFlatZincOptions::FZ_SEARCH_EPS_GRID_COMPUTATION) {
        MyFlatZincSpace* bestF = static_cast<MyFlatZincSpace*>(best);

        MyLockFile lockF(optSearch.obj_file);
        lockF.lock();
        char data[21]; //=> 2^64 = 18446744073709551616 (20 figures)
        int rd = lockF.read(data, 21);
        if(rd) {
            data[rd] = '\0';
            int currentObj = ::atoi(data);
            if (method == MyFlatZincSpace::MIN) {
                if(valueObj < currentObj) {
                    std::string numStr(Convert2String(valueObj));

                    lockF.write((void*)numStr.c_str(), numStr.size());
                    lockF.unlock();

                    return valueObj;
                } else if(valueObj > currentObj) {

                    lockF.unlock();

                    Gecode::rel(*best, bestF->iv[bestF->optVar()], Gecode::IRT_LE,
                                currentObj);
                    (void) best->status();
                    return currentObj;
                }
            } else if(method == MyFlatZincSpace::MAX) {
                if(valueObj > currentObj) {
                    std::string numStr(Convert2String(valueObj));

                    lockF.write((void*)numStr.c_str(), numStr.size());
                    lockF.unlock();

                    return valueObj;
                } else if(valueObj < currentObj) {

                    lockF.unlock();

                    Gecode::rel(*best, bestF->iv[bestF->optVar()], Gecode::IRT_GR,
                                currentObj);
                    (void) best->status();
                    return currentObj;
                }
            }
            //rd == 0 => no read
            //
        } else {
            std::string numStr(Convert2String(valueObj));
            lockF.write((void*)numStr.c_str(), numStr.size());
        }
        lockF.unlock();
    }
    return valueObj;
}


forceinline void
EPS_BAB::solution(Worker* w) {
    m_search.acquire();

    if (w) {
        if(w->best) {
            if(best) {
                MyFlatZincSpace* s_f = static_cast<MyFlatZincSpace*>(w->best);
                MyFlatZincSpace* s_b = static_cast<MyFlatZincSpace*>(best);

                if (s_b->method() == MyFlatZincSpace::MIN) {
                    if(s_f->iv[s_f->optVar()].max() < s_b->iv[s_b->optVar()].max()) {
                        delete best;
                        best = w->best->clone();

                        int bestObj = notifyBestSolution(s_f->iv[s_f->optVar()].max(), MyFlatZincSpace::MIN);

                        //update worker best solution
                        if(bestObj != s_f->iv[s_f->optVar()].max()) {
                            w->better(best);
                        }

                    } else if(s_f->iv[s_f->optVar()].max() == s_b->iv[s_b->optVar()].max()) {
                        m_search.release();
                        return;
                    } else {
                        w->better(best);
                        //delete w->best;
                        //w->best = best->clone(false);
                        m_search.release();
                        return;
                    }
                } else if (s_b->method() == MyFlatZincSpace::MAX) {
                    if(s_f->iv[s_f->optVar()].min() > s_b->iv[s_b->optVar()].min()) {
                        delete best;
                        best = w->best->clone();

                        int bestObj = notifyBestSolution(s_f->iv[s_f->optVar()].min(), MyFlatZincSpace::MAX);

                        //update worker best solution
                        if(bestObj != s_f->iv[s_f->optVar()].min()) {
                            w->better(best);
                        }

                    } else if(s_f->iv[s_f->optVar()].min() == s_b->iv[s_b->optVar()].min()) {
                        m_search.release();
                        return;
                    } else {
                        w->better(best);
                        //delete w->best;
                        //w->best = best->clone(false);
                        m_search.release();
                        return;
                    }
                } else {
                    std::cerr << "unknow optimization" << std::endl;
                    m_search.release();
                    return;
                }
            } else {
                //delete best;
                best = w->best ? w->best->clone() : NULL;
            }
        } else {
            //delete w->best;
            w->better(best);
            m_search.release();
            return;
            //delete w->best;
            //w->best = best->clone(false);
            //w->best = best ? best->clone(false) : NULL;
        }
    } else {
        m_search.release();
        return;
    }

    if(!best) {
        m_search.release();
        return;
    }
#ifdef _DEBUG
    std::cerr << "new objective: " << static_cast<MyFlatZincSpace*>(best)->iv[static_cast<MyFlatZincSpace*>(best)->optVar()].val() << std::endl;
#endif

    bool bs = signal();
    solutions.push(best->clone());
    if (bs) {
        e_search.signal();
    }

    m_search.release();

}


/*
 * Worker: finding and stealing working
 */
forceinline void
EPS_BAB::Worker::find(void) {


    if(mode_search == DECOMPOSITION) {

        engine().lockFindJobDecomposition();

        unsigned int index_tuple_last = engine()._current_index_tuple_decomposition + engine()._master->_group_tuples[engine()._current_problem_decomposition];
        /*
        std::cerr << engine()._group_tuples.size() << std::endl;
        std::cerr << engine()._current_index_tuple_decomposition << std::endl;
        std::cerr << index_tuple_last << std::endl;
        */
        if(this->_tuples_bool_ndi) {
            delete this->_tuples_bool_ndi;
        }
        this->_tuples_bool_ndi = new Gecode::TupleSet();
        if(engine()._master->_tuples_bool_ndi->tuples()) {
            int arity_bool = engine()._master->_tuples_bool_ndi->arity();
            for(unsigned int i = engine()._current_index_tuple_decomposition; i < index_tuple_last; i++) {
                Gecode::IntArgs tuple(arity_bool, (*engine()._master->_tuples_bool_ndi)[i]);
                /*
                std::cerr << "tuple[";
                for(int j = 0; j < tuple.size(); j++) {
                	std::cerr << tuple[j] << ", ";
                }
                std::cerr << "]\n";
                */
                this->_tuples_bool_ndi->add(tuple);
            }
        }
        this->_tuples_bool_ndi->finalize();


        if(this->_tuples_int_ndi) {
            delete this->_tuples_int_ndi;
        }
        this->_tuples_int_ndi = new Gecode::TupleSet();

        if(engine()._master->_tuples_int_ndi->tuples()) {
            int arity_int = engine()._master->_tuples_int_ndi->arity();
            for(unsigned int i = engine()._current_index_tuple_decomposition; i < index_tuple_last; i++) {
                Gecode::IntArgs tuple(arity_int, (*engine()._master->_tuples_int_ndi)[i]);
                /*
                std::cerr << "tuple[";
                for(int j = 0; j < tuple.size(); j++) {
                	std::cerr << tuple[j] << ", ";
                }
                std::cerr << "]\n";
                */
                this->_tuples_int_ndi->add(tuple);
            }
        }
        this->_tuples_int_ndi->finalize();

        //Lock this instruction because concurrent access to space hook must be protected
        MyFlatZincSpace* space_for_decomposition = static_cast<MyFlatZincSpace*>(engine()._space_home->_space_hook->clone(false));

        //delete best;
        engine().solution(this);

        if (best) {
            space_for_decomposition->constrain(*best);
            space_for_decomposition->status();
        }


        engine()._current_index_tuple_decomposition = index_tuple_last;
        engine()._current_problem_decomposition++;

        engine().unlockFindJobDecomposition();

        MySearchOptions opt(engine().optSearch);
        //std::cerr << this->id << "\n";
        //std::cerr << engine()._problems_to_generate_worker.size() << "\n";
        opt.nb_problems = engine()._problems_for_decomposition[this->id];

        space_for_decomposition->_nodes_decomposition = 0;
        space_for_decomposition->_fails_decomposition = 0;
        space_for_decomposition->_memory_decomposition = 0;
        space_for_decomposition->_iterations_decomposition = 0;
        space_for_decomposition->_depth_decomposition = 0;

        this->decomposeProblems(space_for_decomposition, opt);

        engine().lockFindJobResolution();

        if(_group_tuples.size()) {

            engine()._groups_tuples_resolution.insert(engine()._groups_tuples_resolution.end(), _group_tuples.begin(), _group_tuples.end());
            engine()._tuples_bool_resolution.push_back(new Gecode::TupleSet(*_tuples_bool_ndi));
            engine()._tuples_int_resolution.push_back(new Gecode::TupleSet(*_tuples_int_ndi));

            if(!engine()._tuples_bool_resolution.back()->finalized()) {
                engine()._tuples_bool_resolution.back()->finalize();
            }

            if(!engine()._tuples_int_resolution.back()->finalized()) {
                engine()._tuples_int_resolution.back()->finalize();
            }

            engine()._space_home->_nodes_decomposition += space_for_decomposition->_nodes_decomposition;
            engine()._space_home->_fails_decomposition += space_for_decomposition->_fails_decomposition;
            engine()._space_home->_memory_decomposition += space_for_decomposition->_memory_decomposition;
            engine()._space_home->_iterations_decomposition += space_for_decomposition->_iterations_decomposition;

            if(engine()._space_home->_depth_decomposition < space_for_decomposition->_depth_decomposition) {
                engine()._space_home->_depth_decomposition = space_for_decomposition->_depth_decomposition;
            }

            engine()._space_home->_problems = engine()._groups_tuples_resolution.size();
        }

        engine()._nb_workers_decomposition_done++;

#ifdef _DEBUG
        if(_group_tuples.size()) {
            fprintf(stderr, "Decomposition by worker %d done => %d problems generated\n", this->id, _group_tuples.size());
        } else {
            fprintf(stderr, "Decomposition by worker %d done => 0 problems generated (UNSAT)\n", this->id);
        }
#endif

        if(engine()._nb_workers_decomposition_done == engine().workers()) {
            engine()._space_home->_time_decomposition = static_cast<unsigned int>(floor(engine()._timer_decomposition.stop()));
#ifdef _DEBUG
            fprintf(stderr, "Decomposition by all workers done\n");
#endif
        }

        engine().unlockFindJobResolution();

        delete space_for_decomposition;

        delete _tuples_bool_ndi;
        _tuples_bool_ndi = NULL;
        delete _tuples_int_ndi;
        _tuples_int_ndi = NULL;
        _group_tuples.clear();

        mode_search = RESOLUTION;
    }

    engine().lockFindJobResolution();

    if(engine()._current_problem_resolution < engine()._groups_tuples_resolution.size()) {

        _timer_problem.start();

        if(engine().optSearch.mode_search == MyFlatZincOptions::FZ_SEARCH_EPS_GRID_GENERATION) {
            std::string file_problem(*engine()._space_home->_name_instance + "_sp_" + Convert2String(engine()._current_problem) + ".txt");

            std::ofstream os(file_problem);
            if (!os.good()) {
                std::cerr << "Could not open file " << file_problem << " for output."
                          << std::endl;
                exit(EXIT_FAILURE);
            }


            unsigned int index_tuple_last = engine()._current_index_tuple_resolution + engine()._groups_tuples_resolution[engine()._current_problem_resolution];

            Gecode::TupleSet& bool_tuples = *engine()._tuples_bool_resolution[engine()._current_index_group_tuple_resolution];
            int nb_bool_tuples = bool_tuples.tuples();
            if(nb_bool_tuples) {
                int arity_bool = bool_tuples.arity();

                //std::cerr << "bv[";
                os << "v ";
                for(int i = 0; i < arity_bool; i++) {
                    os << i << " ";
                    //    std::cerr << i << ", ";
                }
                //std::cerr << "]\n";
                os << "\n";

                for(unsigned int i = engine()._current_index_tuple_resolution; i < index_tuple_last; i++) {
                    Gecode::IntArgs tuple(arity_bool, bool_tuples[i]);

                    os << "t ";
                    for(int j = 0; j < tuple.size(); j++) {
                        os << tuple[j] << " ";
                    }
                    os << "\n";


                }

            }

            Gecode::TupleSet& int_tuples = *engine()._tuples_int_resolution[engine()._current_index_group_tuple_resolution];
            int nb_int_tuples = int_tuples.tuples();
            if(nb_int_tuples) {
                int arity_int = int_tuples.arity();

                //std::cerr << "iv[";
                os << "v ";
                for(int i = 0; i < arity_int; i++) {
                    os << i + engine()._space_home->bv.size() << " ";
                    //    std::cerr << i << ", ";
                }
                //std::cerr << "]\n";
                os << "\n";

                for(unsigned int i = engine()._current_index_tuple_resolution; i < index_tuple_last; i++) {
                    Gecode::IntArgs tuple(arity_int, int_tuples[i]);

                    os << "t ";
                    for(int j = 0; j < tuple.size(); j++) {
                        os << tuple[j] << " ";
                    }
                    os << "\n";
                    /*
                    std::cerr << "tuple[";
                    for(int j = 0; j < tuple.size(); j++) {
                    	std::cerr << tuple[j] << ", ";
                    }
                    std::cerr << "]\n";
                    */
                }
            }


            if(index_tuple_last+1 > std::max(nb_int_tuples, nb_bool_tuples)) {

                engine()._current_index_tuple_resolution = 0;
                engine()._current_index_group_tuple_resolution++;

            } else {
                engine()._current_index_tuple_resolution = index_tuple_last;
            }

            engine()._current_problem_resolution++;

            os.close();
            idle = true;
            engine()._current_problem++;

        } else {
            idle = false;
            mark = d = 0;

            MyFlatZincSpace* space_resolution = static_cast<MyFlatZincSpace*>(engine()._space_home->clone(false)); //cloning for independant worker



            unsigned int index_tuple_last = engine()._current_index_tuple_resolution + engine()._groups_tuples_resolution[engine()._current_problem_resolution];

            Gecode::TupleSet& bool_tuples = *engine()._tuples_bool_resolution[engine()._current_index_group_tuple_resolution];
            int nb_bool_tuples = bool_tuples.tuples();
            if(nb_bool_tuples) {
                int arity_bool = bool_tuples.arity();
                Gecode::TupleSet tuples;
                for(unsigned int i = engine()._current_index_tuple_resolution; i < index_tuple_last; i++) {
                    Gecode::IntArgs tuple(arity_bool, bool_tuples[i]);

                    /*
                    std::cerr << "tuple[";
                    for(int j = 0; j < tuple.size(); j++) {
                    	std::cerr << tuple[j] << ", ";
                    }
                    std::cerr << "]\n";
                    */
                    tuples.add(tuple);

                }
                tuples.finalize();

                Gecode::BoolVarArgs vars(arity_bool);
                //std::cerr << "vars[";
                for(int i = 0; i < arity_bool; i++) {
                    vars[i] = space_resolution->bv[i];
                    //    std::cerr << j << ", ";
                }

                //std::cerr << "]\n";
                //std::cerr << "decision variables : " << s->args_iv.size() << std::endl;
                //if(nb_bool_tuples == 1) {
                //    for(int i = 0; i < arity_bool; i++) {
                //        Gecode::rel(*space_resolution, vars[i], Gecode::IRT_EQ, tuples[0][i]);
                //    }
                //
                //} else {
                Gecode::extensional(*space_resolution, vars, tuples, Gecode::EPK_DEF, Gecode::ICL_DOM);
                //}
            }
            //std::cerr << "Problem " << engine()._current_problem_resolution << "\n";

            Gecode::TupleSet& int_tuples = *engine()._tuples_int_resolution[engine()._current_index_group_tuple_resolution];
            int nb_int_tuples = int_tuples.tuples();
            if(nb_int_tuples) {
                int arity_int = int_tuples.arity();

                Gecode::TupleSet tuples;
                for(unsigned int i = engine()._current_index_tuple_resolution; i < index_tuple_last; i++) {
                    Gecode::IntArgs tuple(arity_int, int_tuples[i]);
                    /*
                    std::cerr << "tuple[";
                    for(int j = 0; j < tuple.size(); j++) {
                    	std::cerr << tuple[j] << ", ";
                    }
                    std::cerr << "]\n";
                    */
                    tuples.add(tuple);
                }
                tuples.finalize();
                /*
                const int nb_tuples = tuples.tuples();

                int* values_vertical = new int[nb_tuples];
                for(unsigned int i = 0; i < tuples.arity(); i++) {

                	for(unsigned int j = 0; j < nb_tuples; j++) {
                		values_vertical[j] = tuples[j][i];
                	}
                	Gecode::IntSet values(values_vertical, nb_tuples);

                	//Gecode::IntVar x(*space_resolution, values);

                	Gecode::dom(*space_resolution, space_resolution->iv[i], values);

                	//std::cerr << space_resolution->iv[i] << " (before) => ";
                	//space_resolution->iv[i].update(*space_resolution, false, x);


                	//Gecode::rel(*space_resolution, space_resolution->iv[i], Gecode::IRT_GQ, space_resolution->iv[i].min());
                	//Gecode::rel(*space_resolution, space_resolution->iv[i], Gecode::IRT_LQ, space_resolution->iv[i].max());
                }
                delete [] values_vertical;
                */
                //for(unsigned int i = 0; i < tuples.arity(); i++) {
                //    std::cerr << space_resolution->iv[i] << " (after)\n";
                //}


                Gecode::IntVarArgs vars(arity_int);
                //std::cerr << "iv[";
                for(int i = 0; i < arity_int; i++) {
                    vars[i] = space_resolution->iv[i];
                    //    std::cerr << i << ", ";
                }

                //std::cerr << "]\n\n";

                //std::cerr << "decision variables : " << s->args_iv.size() << std::endl;
                //if(nb_int_tuples == 1) {
                //    for(int i = 0; i < arity_int; i++) {
                //        Gecode::rel(*space_resolution, vars[i], Gecode::IRT_EQ, tuples[0][i]);
                //    }

                //} else {
                Gecode::extensional(*space_resolution, vars, tuples, Gecode::EPK_DEF, Gecode::ICL_DOM);
                //}
            }

            //Gecode::branch(*s, s->iv, Gecode::INT_VAR_NONE(), Gecode::INT_VAL_MIN());
            //Gecode::branch(*s, s->iv, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_NONE()), Gecode::INT_VALUES_MIN());
            /* Problem with args :S
            if(s->args_iv.size()) {
            	for(int i = 0; i < s->args_iv.size(); i++) {
            		Gecode::branch(*s, s->args_iv[i], s->branch_var_iv[i], s->branch_val_iv[i]);
            	}
            }
            if(s->args_bv.size()) {
            	Gecode::branch(*s, s->args_bv, s->branch_var_bv, s->branch_val_bv);
            }
            if(s->args_sv.size()) {
            	Gecode::branch(*s, s->args_sv, s->branch_var_sv, s->branch_val_sv);
            }
            if(s->args_fv.size()) {
            	Gecode::branch(*s, s->args_fv, s->branch_var_fv, s->branch_val_fv);
            }
            */

            if(index_tuple_last+1 > std::max(nb_int_tuples, nb_bool_tuples)) {
                //delete engine()._tuples_bool_resolution[engine()._current_index_group_tuple_resolution];
                //delete engine()._tuples_int_resolution[engine()._current_index_group_tuple_resolution];

                //engine()._tuples_bool_resolution[engine()._current_index_group_tuple_resolution] = NULL;
                //engine()._tuples_int_resolution[engine()._current_index_group_tuple_resolution] = NULL;

                engine()._current_index_tuple_resolution = 0;
                engine()._current_index_group_tuple_resolution++;

            } else {
                engine()._current_index_tuple_resolution = index_tuple_last;
            }

            engine()._current_problem_resolution++;


            if(cur) {
                delete cur;
            }

            cur = space_resolution;

            if (best) {
                cur->constrain(*best);

            }



            engine()._current_problem++;
        }

    } else if(engine()._nb_workers_decomposition_done == engine().workers()) {

        // Report that worker is idle
        if(!done) {
            engine().idle();


            done = true;

            //FINISH
            if(engine().getBusyWorkers() == 0) {
                if(engine().workers() > 1) {
                    engine()._space_home->_time_max_inactivity = static_cast<unsigned int>(floor(engine().timer_max_inactivity.stop()));
                }

                //Create obj file
                if(engine().optSearch.mode_search == MyFlatZincOptions::FZ_SEARCH_EPS_GRID_GENERATION) {
                    std::string file_obj(*engine()._space_home->_name_instance + "_obj.txt");
                    std::ofstream os(file_obj);

                    //std::ofstream os(engine().optSearch.obj_file);
                    if (!os.good()) {
                        std::cerr << "Could not open file " << engine().optSearch.obj_file << " for creating objective."
                                  << std::endl;
                        exit(EXIT_FAILURE);
                    }

                    MyFlatZincSpace* bestF = engine()._space_home;
                    if(engine().best) {
                        bestF = static_cast<MyFlatZincSpace*>(engine().best);
                    }
                    if(bestF->method() == MyFlatZincSpace::MIN) {
                        os << bestF->iv[bestF->optVar()].max();
                    } else if(bestF->method() == MyFlatZincSpace::MAX) {
                        os << bestF->iv[bestF->optVar()].min();
                    }

                    os.close();
                    //std::cerr << "create Objective file " << engine().optSearch.obj_file << std::endl;
                }
            }

            if(engine().workers() > 1 && !engine()._already_timer_max_inactivity_started) {
                engine().timer_max_inactivity.start();
                engine()._already_timer_max_inactivity_started = true;
            }
        }

    }
    engine().unlockFindJobResolution();

}


/*
 * Statistics
 */
Gecode::Search::Statistics
EPS_BAB::statistics(void) const {
    Gecode::Search::Statistics s;
    if(_workers) {
        for (unsigned int i=0; i<workers(); i++)
            s += worker(i)->statistics();
    }
    return s;
}

/*
 * Actual work
 */
void
EPS_BAB::Worker::run(void) {
    // Peform initial delay, if not first worker
    //if (this != engine().worker(0))
    //    Gecode::Support::Thread::sleep(Gecode::Search::Config::initial_delay);
    // Okay, we are in business, start working
    while (true) {
        switch (engine().cmd()) {
        case C_WAIT:
            // Wait
            engine().wait();
            break;
        case C_TERMINATE:
            // Acknowledge termination request
            engine().ack_terminate();
            // Wait until termination can proceed
            engine().wait_terminate();
            // Terminate thread
            engine().terminated();
            return;
        case C_WORK:
            // Perform exploration work
        {
            if(!done) {
                //m.acquire();
                if (idle) {
                    //m.release();
                    // Try to find new work
                    find();
                } else if (cur != NULL) {
                    start();
                    if (stop(engine().opt())) {
                        // Report stop
                        engine().stop();
                    } else {

                        node++;
                        switch (cur->status(*this)) {

                        case Gecode::SS_FAILED:
                            fail++;
                            delete cur;
                            cur = NULL;

                            break;
                        case Gecode::SS_SOLVED: {
                            // Deletes all pending branchers
                            (void) cur->choice();

                            best = cur;

                            cur = NULL;
                        }
                        break;
                        case Gecode::SS_BRANCH: {
                            Gecode::Space* c;

                            if ((d == 0) || (d >= engine().opt().c_d)) {

                                c = cur->clone();
                                d = 1;
                            } else {
                                c = NULL;
                                d++;
                            }

                            const Gecode::Choice* ch = path.push(*this,cur,c);
                            cur->commit(*ch,0);
                        }
                        break;
                        default:
                            GECODE_NEVER;
                        }
                    }
                } else if (path.next()) {
                    cur = path.recompute(d, engine().opt().a_d,*this);

                    if(cur && best) {
                        cur->constrain(*best);
                    }


                } else {
                    idle = true;

                    engine().solution(this);
                    //add timer for finished a subproblem
                    engine().notifyFinishedSubproblem(this->id, _timer_problem.stop());

                }
            }
        }
        break;
        default:
            GECODE_NEVER;
        }
    }
}


/*
 * Termination and deletion
 */
EPS_BAB::Worker::~Worker(void) {
    delete best;
}

EPS_BAB::~EPS_BAB(void) {

    if(_workers) {
        terminate();
        Gecode::heap.rfree(_workers);
    }

    if(best) {
        delete best;
    }

    if(_master) {
        delete _master;
    }

    STLDeleteElements(&this->_tuples_bool_resolution);
    STLDeleteElements(&this->_tuples_int_resolution);

}

///decomposeProblems
void EPS_BAB::Worker::decomposeProblems(MyFlatZincSpace* s, const MySearchOptions& o) {
    _group_tuples.clear();


    MySearchOptions opt = o;
    opt.threads = o.threads;
    opt.clone = false;

    unsigned int P = o.nb_problems;
#ifdef _DEBUG
    std::cerr << "expected problems : " << P << std::endl;
#endif
    unsigned int nb_int_decision_variables = s->iv.size();
    unsigned int nb_bool_decision_variables = s->bv.size();

    unsigned int nb_decision_variables = nb_int_decision_variables + nb_bool_decision_variables;

    if(nb_decision_variables == 0) {
        std::cerr << "No decision variables for decomposition : " << std::endl;
        return;
    }

    unsigned int product_domain = 1;
    unsigned int level = 0;


    unsigned int nb_solutions = 0;

    BoundedBAB dbdfs(NULL, opt);

    s->_iterations_decomposition = 0;

    /// Queue of solutions

    std::list<MyFlatZincSpace*> sub_problems;
    std::list<MyFlatZincSpace*> tmp_sub_problems;

    Gecode::StatusStatistics sstat;
    Gecode::SpaceStatus ss;

    do {

        s->_iterations_decomposition++;

        level = 0;
        /*
        delete dbdfs.best;
        dbdfs.best = NULL;
        delete dbdfs.cur;
        dbdfs.cur = NULL;
        */
        /* IMPORTANT FIXME when space is Gecode::FAILED */

        if(s->failed()) {
            return;
        }

        MyFlatZincSpace* space_work = static_cast<MyFlatZincSpace*>(s->clone());

        if(nb_bool_decision_variables && _tuples_bool_ndi && _tuples_bool_ndi->tuples()) {

            Gecode::BoolVarArgs vars_ndi;
            for(int i = 0; i < _tuples_bool_ndi->arity(); i++) {
                vars_ndi << space_work->bv[i];
            }

            //Add table constraint
            Gecode::extensional(*space_work, vars_ndi, *_tuples_bool_ndi);

            level += _tuples_bool_ndi->arity();
        }

        if(nb_int_decision_variables && _tuples_int_ndi && _tuples_int_ndi->tuples()) {

            Gecode::IntVarArgs vars_ndi;
            for(int i = 0; i < _tuples_int_ndi->arity(); i++) {
                vars_ndi << space_work->iv[i];
            }

            //Add table constraint
            Gecode::extensional(*space_work, vars_ndi, *_tuples_int_ndi);

            level += _tuples_int_ndi->arity();
        }

        //m.acquire();
        if(best) {
            space_work->constrain(*best);
        }
        //m.release();

        dbdfs.reset(space_work);

        /*
        Gecode::StatusStatistics sstat;
        Gecode::SpaceStatus ss = space_work->status(sstat);
        */

        /*
        for (unsigned int i = 0; i < space_work->iv.size(); i++) {
            std::cerr << i << " => sizeIntVar : " << space_work->iv[i].size() << std::endl;
        }

        for (unsigned int i = 0; i < space_work->bv.size(); i++) {
            std::cerr << i << " => sizeBoolVar : " << space_work->bv[i].size() << std::endl;
        }
        */

        //Release tuples
        //
        delete _tuples_int_ndi;
        _tuples_int_ndi = new Gecode::TupleSet();

        delete _tuples_bool_ndi;
        _tuples_bool_ndi = new Gecode::TupleSet();

        if(space_work) {

            unsigned int old_level = level;

            for(unsigned int i = level; i < nb_decision_variables; i++) {

                level++;

                if(i < nb_bool_decision_variables) {
                    product_domain *= space_work->bv[i].size();
                    if(product_domain * space_work->bv[i].size() > P) {
                        break;
                    }
                } else {
                    product_domain *= space_work->iv[i - nb_bool_decision_variables].size();
                    if(product_domain * space_work->iv[i - nb_bool_decision_variables].size() > P) {
                        break;
                    }
                }
                //std::cerr << "product domain : " << product_domain << std::endl;
            }

            if(level) {
                Gecode::BoolVarArgs vars_bool_wanted_visited;
                Gecode::IntVarArgs vars_int_wanted_visited;
                for(unsigned int i = old_level; i < level; i++) {
                    //std::cerr << "On va assigner la variable : " << i << std::endl;
                    if(i < nb_bool_decision_variables) {
                        vars_bool_wanted_visited << space_work->bv[i];
                    } else {
                        vars_int_wanted_visited << space_work->iv[i - nb_bool_decision_variables];
                    }
                }

                if(vars_bool_wanted_visited.size()) {
                    Gecode::branch(*space_work, vars_bool_wanted_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_NONE()), Gecode::INT_VALUES_MIN());
                    //Gecode::branch(*space_work, vars_bool_wanted_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_SIZE_MIN()), Gecode::INT_VALUES_MIN());
                }

                if(vars_int_wanted_visited.size()) {
                    Gecode::branch(*space_work, vars_int_wanted_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_NONE()), Gecode::INT_VALUES_MIN());
                    //Gecode::branch(*space_work, vars_int_wanted_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_SIZE_MIN()), Gecode::INT_VALUES_MIN());
                }

            }
            if(old_level) {
                Gecode::BoolVarArgs vars_bool_already_visited;
                Gecode::IntVarArgs vars_int_already_visited;
                for(unsigned int i = 0; i < old_level; i++) {
                    //std::cerr << "On va assigner la variable : " << i << std::endl;
                    if(i < nb_bool_decision_variables) {
                        vars_bool_already_visited << space_work->bv[i];
                    } else {
                        vars_int_already_visited << space_work->iv[i - nb_bool_decision_variables];
                    }
                }

                if(vars_bool_already_visited.size()) {
                    Gecode::branch(*space_work, vars_bool_already_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_NONE()), Gecode::INT_VALUES_MIN());
                    //Gecode::branch(*space_work, vars_bool_already_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_SIZE_MIN()), Gecode::INT_VALUES_MIN());
                }

                if(vars_int_already_visited.size()) {
                    Gecode::branch(*space_work, vars_int_already_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_NONE()), Gecode::INT_VALUES_MIN());
                    //Gecode::branch(*space_work, vars_int_already_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_SIZE_MIN()), Gecode::INT_VALUES_MIN());
                }
            }

            //int nb_tuples = 0;

            //Do not forget if stat(space_work) == SS_FAILED
            //Gecode::IntArgs tuple(level > nb_int_decision_variables ? nb_int_decision_variables - level : level);
            //Gecode::IntArgs tuple(level);

            //Bug fixed in loop solution
            //m.acquire();
            MyFlatZincSpace* solution = static_cast<MyFlatZincSpace*>(dbdfs.next());
            //m.release();
            while(solution) {
                /*
                std::cerr << "IntVar " << std::endl;
                for (unsigned int i = 0; i < solution->iv.size(); i++) {
                    if(solution->iv[i].assigned()) {
                        std::cerr << i << " : " << solution->iv[i].val() << std::endl;
                    } else  {
                        std::cerr << i << " : pas d'assignement" << std::endl;
                    }
                }


                std::cerr << "BoolVar " << std::endl;
                for (unsigned int i = 0; i < solution->bv.size(); i++) {
                    if(solution->bv[i].assigned()) {
                        std::cerr << i << " : " << solution->bv[i].val() << std::endl;
                    } else  {
                        std::cerr << i << " : pas d'assignement" << std::endl;
                    }
                }
                */
                bool isSolution = solution->iv[solution->optVar()].assigned();
                if(isSolution) {
                    for (int i = 0; i < solution->iv.size(); ++i) {
                        if(!solution->iv[i].assigned()) {
                            isSolution = false;
                            break;
                        }
                    }

                    if(isSolution) {
                        for (int i = 0; i < solution->bv.size(); ++i) {
                            if(!solution->bv[i].assigned()) {
                                isSolution = false;
                                break;
                            }
                        }
                    }
                }

                if(isSolution) {

                    //nb_tuples = 0;
                    delete best;
                    best = solution->clone(false);
                    //this mehod set the new best solution
                    engine().solution(this);

                    nb_solutions++;

                    //m.acquire();


#ifdef _DEBUG
                    std::cerr << "Best solution found in dfs decomposition\n";
                    std::cerr << "objective : " << solution->iv[solution->optVar()] << std::endl;
#endif
                    delete dbdfs.best; //== solution
                    dbdfs.best = best->clone();


                    //Gecode::Support::DynamicQueue<MyFlatZincSpace*, Gecode::Heap> tmp_sub_problems(Gecode::heap);
                    //std::queue<MyFlatZincSpace*> tmp_sub_problems;
                    while(!sub_problems.empty()) {
                        //MyFlatZincSpace* sb_problem = sub_problems.pop();
                        MyFlatZincSpace* sb_problem = sub_problems.back();
                        sub_problems.pop_back();

                        sb_problem->constrain(*best);
                        ss = sb_problem->status(sstat);

                        if(ss == Gecode::SS_FAILED) {
                            delete sb_problem;
                        } else {
                            tmp_sub_problems.push_back(sb_problem);
                        }
                    }
                    //m.release();

                    while(!tmp_sub_problems.empty()) {
                        //sub_problems.push(tmp_sub_problems.pop());
                        sub_problems.push_back(tmp_sub_problems.back());
                        tmp_sub_problems.pop_back();
                    }

                } else {

                    //nb_tuples++;
                    sub_problems.push_back(solution);
                    //std::cerr << nb_tuples << std::endl;
                    //Important : clone space_work because it is deleted in BoundedBAB
                    //m.acquire();
                    if(best) {
                        dbdfs.best = best->clone();
                    } else {
                        dbdfs.best = s->clone();
                    }
                    //m.release();

                }
                //m.acquire();
                solution = static_cast<MyFlatZincSpace*>(dbdfs.next());
                //m.release();

            }

//#ifdef _DEBUG

            //std::stable_sort(sub_problems.begin(), sub_problems.end(), ProblemCompare(true));
//#endif

            while(!sub_problems.empty()) {
                //MyFlatZincSpace* sb_problem = sub_problems.pop();
                MyFlatZincSpace* sb_problem = sub_problems.front();
                sub_problems.pop_front();
#ifdef _DEBUG
                std::cerr << sb_problem->iv[sb_problem->optVar()] << std::endl;
#endif

                Gecode::IntArgs tuple_int;
                Gecode::IntArgs tuple_bool;

                //no add the objective val int tuple
                for (unsigned int i = 0; i < level; i++) {
                    //std::cerr << i+1 << " : " << solution->iv[i].val() << std::endl;
                    if(i < nb_bool_decision_variables) {
                        tuple_bool << sb_problem->bv[i].val();
                    } else {
                        tuple_int << sb_problem->iv[i - nb_bool_decision_variables].val();
                    }
                }

                //Version no collapse last level
                if(tuple_bool.size()) {
                    _tuples_bool_ndi->add(tuple_bool);
                }

                if(tuple_int.size()) {
                    _tuples_int_ndi->add(tuple_int);
                }

                //Version with collapse last level
                /*
                if(level < nb_decision_variables) {

                    if(level < nb_bool_decision_variables) {
                        tuple_bool << 0;

                        //for(Gecode::BoolVarValues i(sb_problem->bv[level]); i(); i++) {
                        for(int a = sb_problem->bv[level].min(); a <= sb_problem->bv[level].max(); a++) {
                            tuple_bool[tuple_bool.size()-1] = a;
                            _tuples_bool_ndi->add(tuple_bool);
                        }
                    } else {
                        tuple_int << 0;

                        for(Gecode::IntVarValues i(sb_problem->iv[level - nb_bool_decision_variables]); i(); ++i) {
                            //for(int a = sb_problem->iv[level - nb_bool_decision_variables].min(); a <= sb_problem->iv[level - nb_bool_decision_variables].max(); a++) {
                            tuple_int[tuple_int.size()-1] = i.val();
                            _tuples_int_ndi->add(tuple_int);
                        }
                    }
                } else {

                    //nb_tuples++;
                    //std::cerr << "tuple added : " << nb_tuples << std::endl;
                    if(tuple_bool.size()) {
                        _tuples_bool_ndi->add(tuple_bool);
                    }

                    if(tuple_int.size()) {
                        _tuples_int_ndi->add(tuple_int);
                    }
                }
                */
                //Fix memory leak
                delete sb_problem;
            }
        }

        Gecode::Search::Statistics stat = dbdfs.statistics();
        s->_nodes_decomposition = stat.node;
        s->_fails_decomposition = stat.fail;

        if(_tuples_int_ndi) {
            _tuples_int_ndi->finalize();
        }
        if(_tuples_bool_ndi) {
            _tuples_bool_ndi->finalize();
        }


        if(_tuples_int_ndi || _tuples_bool_ndi) {
            product_domain = _tuples_bool_ndi && _tuples_bool_ndi->tuples() ? _tuples_bool_ndi->tuples() : _tuples_int_ndi->tuples();
        } else {
            product_domain = 0;
        }

        level = _tuples_bool_ndi->arity() + _tuples_int_ndi->arity();

        s->_depth_decomposition = level;

#ifdef _DEBUG
        std::cerr << "depth decomposition : " << level << std::endl;
        std::cerr << "number of tuples : " << product_domain << std::endl;
#endif

        if(product_domain == 0) {
            delete _tuples_int_ndi;
            _tuples_int_ndi = NULL;
            delete _tuples_bool_ndi;
            _tuples_bool_ndi = NULL;

#ifdef _DEBUG
            if(nb_solutions > 0) {
                //std::cerr << "Problem SAT detected in problem decomposition\n";
                //std::cerr << "NbSolutions : " << nb_solutions << std::endl;
            } else {
                //std::cerr << "Problem UNSAT detected in dfsbound splitter\n";
            }
#endif
            break;
        }

        int newP = P;

        if(product_domain >= newP) {

            int q = (product_domain / newP) + 1;
            _group_tuples.resize(newP, q);

            size_t sum = newP*q;

            for(size_t i = newP-1; i >=0; i--) {
                if(sum != product_domain) {
                    --_group_tuples[i];
                    --sum;
                } else {
                    break;
                }
            }

            product_domain = newP;

            //STOP
            break;
        } else if(product_domain == newP) {

            _group_tuples.resize(newP, 1);

            //STOP
            break;
        } else if(level == nb_decision_variables) {
            _group_tuples.resize(product_domain, 1);

            //STOP
            break;
        }

    } while(true);
}

/*
   * Create no-goods
   *
   */
Gecode::NoGoods&
EPS_BAB::nogoods(void) {
    Gecode::NoGoods* ng;
    // Grab wait lock for reset
    m_wait_reset.acquire();
    // Release workers for reset
    release(C_RESET);
    // Wait for reset cycle started
    e_reset_ack_start.wait();
    ng = &worker(0)->nogoods();
    // Block workers again to ensure invariant
    block();
    // Release reset lock
    m_wait_reset.release();
    // Wait for reset cycle stopped
    e_reset_ack_stop.wait();
    return *ng;
}


}


//Function to return the EPS_BAB object
Gecode::Search::Engine*
eps_bab_to_engine(Gecode::Space* s, const MySearchOptions& o) {
    MySearchOptions to = o.expand();
    to.a_d = o.a_d;
    to.c_d = o.c_d;
    to.clone = o.clone;
    to.stop = o.stop;
    to.cutoff = to.cutoff;
    to.nb_problems = o.nb_problems;
    to.mode_decomposition = o.mode_decomposition;
    to.mode_search = o.mode_search;
    to.obj_file = o.obj_file;

    //std::cerr << o.nb_problems << std::endl;
    //std::cerr << o.mode_decomposition << std::endl;

    return new Parallel::EPS_BAB(s,to);
}

