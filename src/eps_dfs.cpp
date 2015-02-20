/*---------------------------------------------------------------------------*/
/*                                                                           */
/* eps_dfs.cpp													    */
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

#include <vector>
#include <string>

#include "search.h"
#include "flatzinc.h"
#include "stl_util.h"
#include "lock.h"

using namespace stl_util;

/// Depth-first search engine implementation with limited depth

BoundedDFS::BoundedDFS(Gecode::Space* s, const Gecode::Search::Options& o)
    : Worker(), opt(o), path(static_cast<int>(opt.nogoods_limit)), d(0) {
    if ((s == NULL) || (s->status(*this) == Gecode::SS_FAILED)) {
        fail++;
        cur = NULL;
        if (!opt.clone)
            delete s;
    } else {
        cur = snapshot(s,opt);
    }
}

void
BoundedDFS::reset(Gecode::Space* s) {
    delete cur;
    path.reset();
    d = 0;
    if ((s == NULL) || (s->status(*this) == Gecode::SS_FAILED)) {
        cur = NULL;
    } else {
        cur = s;
    }
    Worker::reset();
}

Gecode::NoGoods&
BoundedDFS::nogoods(void) {
    return path;
}

Gecode::Space*
BoundedDFS::next(void) {
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
            case Gecode::SS_SOLVED: {
                // Deletes all pending branchers
                (void) cur->choice();
                Gecode::Space* s = cur;
                cur = NULL;
                return s;
            }
            case Gecode::SS_BRANCH: {
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
        do {
            if (!path.next())
                return NULL;
            cur = path.recompute(d,opt.a_d,*this);
        } while (cur == NULL);
    }
    GECODE_NEVER;
    return NULL;
}

Gecode::Search::Statistics
BoundedDFS::statistics(void) const {
    return *this;
}


BoundedDFS::~BoundedDFS(void) {
    delete cur;
    path.reset();
}


namespace Parallel {

/// %Parallel depth-first search engine
class EPS_DFS : public Gecode::Search::Parallel::Engine {
protected:
    /// %Parallel depth-first search worker
    class Worker : public Engine::Worker {
    public:
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

        void decomposeProblems(MyFlatZincSpace* s, const MySearchOptions& o);

        void RDFS(MyFlatZincSpace* s, const MySearchOptions& o);

        /// Initialize for space \a s (of size \a sz) with engine \a e
        Worker(Gecode::Space* s, EPS_DFS& e, unsigned int id_worker);
        /// Provide access to engine
        EPS_DFS& engine(void) const;
        /// Start execution of worker
        virtual void run(void);
        /// Try to find some work
        void find(void);
        /// Reset engine to restart at space \a s
        void reset(Gecode::Space* s, int ngdl);
    };
    /// Array of worker references
    Worker** _workers;

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
    void solution(Gecode::Space* s);
    /// Reset engine to restart at space \a s and return new root space
    //TODO See Engine Code
    void reset(Gecode::Space* s) {
        // Grab wait lock for reset
        m_wait_reset.acquire();
        // Release workers for reset
        release(C_RESET);
        // Wait for reset cycle started
        e_reset_ack_start.wait();
        // All workers are marked as busy again
        n_busy = workers();
        for (unsigned int i=0; i<workers(); i++)
            worker(i)->reset(NULL,0);
        // Block workers again to ensure invariant
        block();
        // Release reset lock
        m_wait_reset.release();
        // Wait for reset cycle stopped
        e_reset_ack_stop.wait();
    }
    //@}

    /// \name Engine interface
    //@{
    /// Initialize for space \a s (of size \a sz) with options \a o
    EPS_DFS(Gecode::Space* s, const MySearchOptions& o);
    /// Return statistics
    virtual Gecode::Search::Statistics statistics(void) const;
    /// Return no-goods
    virtual Gecode::NoGoods& nogoods(void);
    /// Destructor
    virtual ~EPS_DFS(void);
    //@}
};


/*
 * Basic access routines
 */
forceinline EPS_DFS&
EPS_DFS::Worker::engine(void) const {
    return static_cast<EPS_DFS&>(_engine);
}
forceinline EPS_DFS::Worker*
EPS_DFS::worker(unsigned int i) const {
    return _workers[i];
}

/*
 * Engine: initialization
 */
forceinline
EPS_DFS::Worker::Worker(Gecode::Space* s, EPS_DFS& e, unsigned int id_worker)
    : Gecode::Search::Parallel::Engine::Worker(s,e),
      done(false),
      mode_search(DECOMPOSITION),
      id(id_worker),
      _tuples_bool_ndi(NULL),
      _tuples_int_ndi(NULL) {
    idle = true;
}

forceinline
EPS_DFS::EPS_DFS(Gecode::Space* s, const MySearchOptions& o)
    : Gecode::Search::Parallel::Engine(o),
      _space_home(static_cast<MyFlatZincSpace*>(s)),
      _already_timer_max_inactivity_started(false),
      _current_problem_decomposition(0),
      _current_index_tuple_decomposition(0),
      _current_index_problem_decomposition(0),
      _current_problem_resolution(0),
      _current_index_tuple_resolution(0),
      _current_index_problem_resolution(0),
      _current_index_group_tuple_resolution(0),
      _current_problem(0),
      _nb_workers_decomposition_done(0),
      _mode_decomposition(PARALLEL), optSearch(o) {

    _workers = NULL;
    _master = new Worker(NULL,*this, -1);

    //Start Timer
    _timer_decomposition.start();

    if(optSearch.mode_decomposition == MyFlatZincOptions::ModeDecomposition::DBDFSwP
            && o.threads > 1) {
        _mode_decomposition = PARALLEL;
    } else {
        _mode_decomposition = SEQUENTIAL;
    }

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
    _master->RDFS(_space_home->_space_hook, optSearch);
    unsigned int time_solve = static_cast<unsigned int>(floor(this->_timer_decomposition.stop()));
    std::cerr << "Time resolution : " << time_solve << std::endl;
    //getchar();
    exit(0);

    _master->decomposeProblems(_space_home->_space_hook, optSearch);

    _master->done = true;

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

/*
 * Statistics
 */
forceinline void
EPS_DFS::Worker::reset(Gecode::Space* s, int ngdl) {
    delete cur;
    path.reset((s != NULL) ? ngdl : 0);
    d = 0;
    idle = false;
    if ((s == NULL) || (s->status(*this) == Gecode::SS_FAILED)) {
        delete s;
        cur = NULL;
    } else {
        cur = s;
    }
    Gecode::Search::Worker::reset();
}

/*
 * Engine: search control
 */
forceinline void
EPS_DFS::solution(Gecode::Space* s) {
    m_search.acquire();
    bool bs = signal();
    solutions.push(s);
    if (bs)
        e_search.signal();
    m_search.release();
}




/*
 * Worker: finding and stealing working
 */
forceinline void
EPS_DFS::Worker::find(void) {

    if(mode_search == DECOMPOSITION) {

        engine().lockFindJobDecomposition();

        unsigned int index_tuple_last = engine()._current_index_tuple_decomposition + engine()._master->_group_tuples[engine()._current_problem_decomposition];

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
            //std::cerr << "nbTuples by worker " << this->id << " : " << std::max(_tuples_bool_ndi->tuples(), _tuples_int_ndi->tuples()) << std::endl;
            /*
            //m.acquire();
            if(best) {
                std::cerr << "best solution " << static_cast<MyFlatZincSpace*>(best)->iv[static_cast<MyFlatZincSpace*>(best)->optVar()].val() << std::endl;
            } else {
                std::cerr << "no best solution " << std::endl;
            }
            //m.release();
            */
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
    //std::cerr << engine()._current_problem_resolution << std::endl;
    //std::cerr << engine()._groups_tuples_resolution.size() << std::endl;
    if(engine()._current_problem_resolution < engine()._groups_tuples_resolution.size()) {

        _timer_problem.start();

        //Generate distribution approach
        //
        if(engine().optSearch.mode_search == MyFlatZincOptions::FZ_SEARCH_EPS_GRID_GENERATION) {
            std::string file_problem(*engine()._space_home->_name_instance + "_sp_" + Convert2String(engine()._current_problem) + ".txt");

            std::ofstream os(file_problem);
            if (!os.good()) {
                std::cerr << "Could not open file " << file_problem << " for output."
                          << std::endl;
                exit(EXIT_FAILURE);
            }


            idle = false;
            d = 0;


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

                Gecode::extensional(*space_resolution, vars, tuples, Gecode::EPK_DEF, Gecode::ICL_DOM);

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



                Gecode::IntVarArgs vars(arity_int);
                //std::cerr << "iv[";
                for(int i = 0; i < arity_int; i++) {
                    vars[i] = space_resolution->iv[i];
                    //    std::cerr << i << ", ";
                }


                Gecode::extensional(*space_resolution, vars, tuples, Gecode::EPK_DEF, Gecode::ICL_DOM);

            }

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



            engine()._current_problem++;
        }
    } else if(engine()._nb_workers_decomposition_done == engine().workers()) {
        //std::cerr << "finish" << std::endl;
        // Report that worker is idle
        if(!done) {
            done = true;
            engine().idle();

            //FINISH
            if(engine().getBusyWorkers() == 0) {
                if(engine().workers() > 1) {
                    engine()._space_home->_time_max_inactivity = static_cast<unsigned int>(floor(engine().timer_max_inactivity.stop()));
                }
            }

            if(engine().workers() > 1 && !engine()._already_timer_max_inactivity_started) {
                engine().timer_max_inactivity.start();
                engine()._already_timer_max_inactivity_started = true;
            }
        }
        //engine().release(C_TERMINATE);
    }

    engine().unlockFindJobResolution();

}

/*
 * Statistics
 */
Gecode::Search::Statistics
EPS_DFS::statistics(void) const {
    Gecode::Search::Statistics s;
    if(_workers) {
        for (unsigned int i=0; i<workers(); i++)
            s += worker(i)->statistics();
    }
    return s;
}


/*
 * Engine: search control
 */
void
EPS_DFS::Worker::run(void) {
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
        case C_RESET:
            // Acknowledge reset request
            engine().ack_reset_start();
            // Wait until reset has been performed
            engine().wait_reset();
            // Acknowledge that reset cycle is over
            engine().ack_reset_stop();
            break;
        case C_WORK:
            // Perform exploration work
        {
            if(!done) {
                if (idle) {
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

                            Gecode::Space* s = cur->clone(false);
                            //Not a good idea to share solution
                            //Gecode::Space* s = cur;

                            delete cur;
                            cur = NULL;
                            engine().solution(s);
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
                    cur = path.recompute(d,engine().opt().a_d,*this);
                } else {
                    idle = true;

                    //add timer for finished a subproblem
                    engine().notifyFinishedSubproblem(this->id, _timer_problem.stop());
                    //path.reset();
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
EPS_DFS::~EPS_DFS(void) {
    if(_workers) {
        terminate();
        Gecode::heap.rfree(_workers);
    }

    if(_master) {
        delete _master;
    }

    STLDeleteElements(&this->_tuples_bool_resolution);
    STLDeleteElements(&this->_tuples_int_resolution);
}

///decomposeProblems
forceinline void
EPS_DFS::Worker::decomposeProblems(MyFlatZincSpace* s, const MySearchOptions& o) {
    _group_tuples.clear();

    MySearchOptions opt(o);
    //std::cerr << "threads : " << o.threads << std::endl;
    opt.threads = o.threads; //No workstealing
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
    //Use DFS for bounded dfs
    BoundedDFS dbdfs(NULL, opt);


    s->_iterations_decomposition = 0;

    do {

        s->_iterations_decomposition++;

        level = 0;

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

        dbdfs.reset(space_work);


        //Release tuples
        //
        delete _tuples_int_ndi;
        _tuples_int_ndi = new Gecode::TupleSet();

        delete _tuples_bool_ndi;
        _tuples_bool_ndi = new Gecode::TupleSet();

        if(space_work) {

            unsigned int old_level = level;

            for(unsigned int i = level; i < nb_decision_variables; i++) {

                if(i < nb_bool_decision_variables) {
                    if(product_domain * space_work->bv[i].size() > P) {
                        break;
                    }
                    product_domain *= space_work->bv[i].size();
                } else {
                    if(product_domain * space_work->iv[i - nb_bool_decision_variables].size() > P) {
                        break;
                    }
                    product_domain *= space_work->iv[i - nb_bool_decision_variables].size();
                }

                level++;
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


            MyFlatZincSpace* solution = static_cast<MyFlatZincSpace*>(dbdfs.next());

            while(solution) {

                bool isSolution = true;
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

                if(isSolution) {

                    engine().solution(solution);
                    nb_solutions++;
                    //std::cerr << "Best solution found in dfs decomposition\n";

                } else {

                    Gecode::IntArgs tuple_int;
                    Gecode::IntArgs tuple_bool;

                    //no add the objective val int tuple
                    for (unsigned int i = 0; i < level; i++) {
                        //std::cerr << i+1 << " : " << solution->iv[i].val() << std::endl;
                        if(i < nb_bool_decision_variables) {
                            tuple_bool << solution->bv[i].val();
                        } else {
                            tuple_int << solution->iv[i - nb_bool_decision_variables].val();
                        }
                    }

                    if(level < nb_decision_variables) {

                        if(level < nb_bool_decision_variables) {
                            tuple_bool << 0;

                            for(int a = solution->bv[level].min(); a <= solution->bv[level].max(); a++) {
                                tuple_bool[tuple_bool.size()-1] = a;
                                _tuples_bool_ndi->add(tuple_bool);
                            }
                        } else {
                            tuple_int << 0;

                            for(Gecode::IntVarValues i(solution->iv[level - nb_bool_decision_variables]); i(); ++i) {

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

                    delete solution;
                }

                solution = static_cast<MyFlatZincSpace*>(dbdfs.next());
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


        if(_tuples_int_ndi && _tuples_bool_ndi) {
            product_domain = _tuples_bool_ndi->tuples() ? _tuples_bool_ndi->tuples() : _tuples_int_ndi->tuples();
        } else {
            product_domain = 0;
        }

        level = _tuples_bool_ndi->arity() + _tuples_int_ndi->arity();

        s->_depth_decomposition = level;

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

        if(product_domain > newP) {

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
EPS_DFS::nogoods(void) {
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

///decomposeProblems
void
EPS_DFS::Worker::RDFS(MyFlatZincSpace* s, const MySearchOptions& o) {

    MySearchOptions opt(o);
    opt.threads = 1;
    opt.clone = false;

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
    BoundedDFS dbdfs(NULL, opt);

    s->_iterations_decomposition = 0;

    do {

        s->_iterations_decomposition++;

        level = 0;
        /*
        delete dbdfs.best;
        dbdfs.best = NULL;
        delete dbdfs.cur;
        dbdfs.cur = NULL;
        */
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
            std::cerr << "Add table constraint" << std::endl;
            std::cerr << "nbTuples : " << _tuples_int_ndi->tuples() << std::endl;
            std::cerr << "arity : " << _tuples_int_ndi->arity() << std::endl;

            level += _tuples_int_ndi->arity();
        }

        dbdfs.reset(space_work);


        //Release tuples
        //
        delete _tuples_int_ndi;
        _tuples_int_ndi = new Gecode::TupleSet();

        delete _tuples_bool_ndi;
        _tuples_bool_ndi = new Gecode::TupleSet();

        if(space_work) {

            unsigned int old_level = level;

            int levels[2] = {opt.first_level, nb_decision_variables};
            level += levels[s->_iterations_decomposition-1];

            if(level == old_level) {
                level++;
            }

            if(level > nb_decision_variables) {
                level = nb_decision_variables;
            }

            std::cerr << "oldlevel : " << old_level << std::endl;
            std::cerr << "newlevel : " << level << std::endl;
            std::cerr << "iteration DFS : " << s->_iterations_decomposition << std::endl;


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
                    //Gecode::branch(*space_work, vars_bool_wanted_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_NONE()), Gecode::INT_VALUES_MIN());
                    Gecode::branch(*space_work, vars_bool_wanted_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_SIZE_MIN()), Gecode::INT_VALUES_MIN());
                }

                if(vars_int_wanted_visited.size()) {
                    //Gecode::branch(*space_work, vars_int_wanted_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_NONE()), Gecode::INT_VALUES_MIN());
                    Gecode::branch(*space_work, vars_int_wanted_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_SIZE_MIN()), Gecode::INT_VALUES_MIN());
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
                    //Gecode::branch(*space_work, vars_bool_already_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_NONE()), Gecode::INT_VALUES_MIN());
                    Gecode::branch(*space_work, vars_bool_already_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_SIZE_MIN()), Gecode::INT_VALUES_MIN());
                }

                if(vars_int_already_visited.size()) {
                    //Gecode::branch(*space_work, vars_int_already_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_NONE()), Gecode::INT_VALUES_MIN());
                    Gecode::branch(*space_work, vars_int_already_visited, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_SIZE_MIN()), Gecode::INT_VALUES_MIN());
                }

                //Gecode::branch(*space_work, space_work->iv, Gecode::TieBreak<Gecode::IntVarBranch>(Gecode::INT_VAR_SIZE_MIN()), Gecode::INT_VALUES_MIN());

            }

            MyFlatZincSpace* solution = static_cast<MyFlatZincSpace*>(dbdfs.next());

            while(solution) {

                bool isSolution = level == nb_decision_variables;

                if(isSolution) {

                    engine().solution(solution);
                    nb_solutions++;
                    //std::cerr << "Best solution found in dfs decomposition\n";

                } else {

                    Gecode::IntArgs tuple_int;
                    Gecode::IntArgs tuple_bool;

                    //no add the objective val int tuple
                    for (unsigned int i = 0; i < level; i++) {
                        //std::cerr << i+1 << " : " << solution->iv[i].val() << std::endl;
                        if(i < nb_bool_decision_variables) {
                            tuple_bool << solution->bv[i].val();
                        } else {
                            tuple_int << solution->iv[i - nb_bool_decision_variables].val();
                        }
                    }

                    if(tuple_bool.size()) {
                        _tuples_bool_ndi->add(tuple_bool);
                    }

                    if(tuple_int.size()) {
                        _tuples_int_ndi->add(tuple_int);
                    }

                    delete solution;
                }

                solution = static_cast<MyFlatZincSpace*>(dbdfs.next());
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


        if(_tuples_int_ndi && _tuples_bool_ndi) {
            product_domain = _tuples_bool_ndi->tuples() ? _tuples_bool_ndi->tuples() : _tuples_int_ndi->tuples();
        } else {
            product_domain = 0;
        }

        level = _tuples_bool_ndi->arity() + _tuples_int_ndi->arity();

        s->_depth_decomposition = level;

        if(product_domain == 0 || level == nb_decision_variables) {
            delete _tuples_int_ndi;
            _tuples_int_ndi = NULL;
            delete _tuples_bool_ndi;
            _tuples_bool_ndi = NULL;

            if(nb_solutions > 0) {
                std::cerr << "Problem SAT detected in problem decomposition\n";
                std::cerr << "NbSolutions : " << nb_solutions << std::endl;
            } else {
                std::cerr << "Problem UNSAT detected in dfsbound splitter\n";
            }

            break;
        }

    } while(true);


}


}


//Function to return the EPS_DFS object
Gecode::Search::Engine*
eps_dfs_to_engine(Gecode::Space* s, const MySearchOptions& o) {
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
    to.first_level = o.first_level;
    //std::cerr << o.nb_problems << std::endl;
    //std::cerr << to.nb_problems << std::endl;

    return new Parallel::EPS_DFS(s,to);
}



