/*---------------------------------------------------------------------------*/
/*                                                                           */
/* main.cpp													                                      */
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

#ifdef _MSC_VER

//#pragma comment(lib, "jemalloc.lib")

/*
#pragma comment(lib, "jemalloc.lib")
#pragma comment(lib, "libtcmalloc_minimal.lib")
#pragma comment(linker, "/INCLUDE:__tcmalloc")
#pragma comment(linker, "/INCLUDE:__tls_used")
*/
//#pragma comment(lib, "libxml2.lib")
//#pragma comment(li0 b, "iconv.lib")
//#pragma comment(lib, "libxml2_a.lib")
//#pragma comment(lib, "iconv_a.lib")



#ifdef _DEBUG


#else


#endif

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "shlwapi.lib")


#ifdef _DEBUG

//rev13366
#pragma comment(lib, "GecodeDriver-4-2-0-d-x86.lib")
#pragma comment(lib, "GecodeFlatZinc-4-2-0-d-x86.lib")
#pragma comment(lib, "GecodeFloat-4-2-0-d-x86.lib")
#pragma comment(lib, "GecodeInt-4-2-0-d-x86.lib")
#pragma comment(lib, "GecodeKernel-4-2-0-d-x86.lib")
#pragma comment(lib, "GecodeMinimodel-4-2-0-d-x86.lib")
#pragma comment(lib, "GecodeSearch-4-2-0-d-x86.lib")
#pragma comment(lib, "GecodeSet-4-2-0-d-x86.lib")
#pragma comment(lib, "GecodeSupport-4-2-0-d-x86.lib")

/*
#pragma comment(lib, "GecodeDriver-3-7-3-d-x86.lib")
#pragma comment(lib, "GecodeFlatZinc-3-7-3-d-x86.lib")
#pragma comment(lib, "GecodeGist-3-7-3-d-x86.lib")
#pragma comment(lib, "GecodeInt-3-7-3-d-x86.lib")
#pragma comment(lib, "GecodeKernel-3-7-3-d-x86.lib")
#pragma comment(lib, "GecodeMinimodel-3-7-3-d-x86.lib")
#pragma comment(lib, "GecodeSearch-3-7-3-d-x86.lib")
#pragma comment(lib, "GecodeSet-3-7-3-d-x86.lib")
#pragma comment(lib, "GecodeSupport-3-7-3-d-x86.lib")
*/

#else

#pragma comment(lib, "GecodeDriver-4-2-0-r-x86.lib")
#pragma comment(lib, "GecodeFlatZinc-4-2-0-r-x86.lib")
#pragma comment(lib, "GecodeFloat-4-2-0-r-x86.lib")
#pragma comment(lib, "GecodeInt-4-2-0-r-x86.lib")
#pragma comment(lib, "GecodeKernel-4-2-0-r-x86.lib")
#pragma comment(lib, "GecodeMinimodel-4-2-0-r-x86.lib")
#pragma comment(lib, "GecodeSearch-4-2-0-r-x86.lib")
#pragma comment(lib, "GecodeSet-4-2-0-r-x86.lib")
#pragma comment(lib, "GecodeSupport-4-2-0-r-x86.lib")

/*
#pragma comment(lib, "GecodeDriver-3-7-3-r-x86.lib")
#pragma comment(lib, "GecodeFlatZinc-3-7-3-r-x86.lib")
#pragma comment(lib, "GecodeGist-3-7-3-r-x86.lib")
#pragma comment(lib, "GecodeInt-3-7-3-r-x86.lib")
#pragma comment(lib, "GecodeKernel-3-7-3-r-x86.lib")
#pragma comment(lib, "GecodeMinimodel-3-7-3-r-x86.lib")
#pragma comment(lib, "GecodeSearch-3-7-3-r-x86.lib")
#pragma comment(lib, "GecodeSet-3-7-3-r-x86.lib")
#pragma comment(lib, "GecodeSupport-3-7-3-r-x86.lib")
*/
#endif


//#pragma comment(lib, "libxml2.lib")

//Faire des tests automatiquement à connaitre
//#include "libcpuid/libcpuid.h"
//#pragma comment(lib, "libcpuid.lib")

#endif

/*
#ifdef _MSC_VER

#include <cstdlib>
#include <new>

#define JEMALLOC_NO_DEMANGLE
#include <jemalloc.h>

#ifndef __CACHE_LINE__
#define __CACHE_LINE__ 64
#endif

//void *operator new(size_t size);
//void operator delete(void *ptr);
//void *operator new(size_t size, const std::nothrow_t& t);
//void operator delete(void *ptr, const std::nothrow_t& t);

#ifndef __THROW
#define __THROW
#endif

void* operator new(size_t size)                  {
    return je_aligned_alloc(__CACHE_LINE__, size);
}
void operator delete(void* p) __THROW            { je_free(p);              }
void* operator new[](size_t size)                {
    return je_aligned_alloc(__CACHE_LINE__, size);
}
void operator delete[](void* p) __THROW          { je_free(p);         }
void* operator new(size_t size, const std::nothrow_t& nt) __THROW {
    return je_aligned_alloc(__CACHE_LINE__, size);
}
void* operator new[](size_t size, const std::nothrow_t& nt) __THROW {
    return je_aligned_alloc(__CACHE_LINE__, size);
}
void operator delete(void* ptr, const std::nothrow_t& nt) __THROW {
    return je_free(ptr);
}
void operator delete[](void* ptr, const std::nothrow_t& nt) __THROW {
    return je_free(ptr);
}



#endif
*/

void fnExit(void) {
}

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <gecode/search.hh>
#include <gecode/flatzinc.hh>
#include <gecode/flatzinc/registry.hh>
#include <gecode/flatzinc/plugin.hh>

#include "flatzinc.h"
#include "models.h"
#include "stl_util.h"
#include "lock.h"

#include "search.h"

//#include <gecode/flatzinc/parser.tab.cpp>
//#include <gecode/flatzinc/lexer.yy.cpp>

using namespace std;
using namespace Gecode;

extern string getBaseName(const string& path);
extern void readSubProblem(MyFlatZincSpace* fg, istream& fin);


#define WORKTAG 1
#define DIETAG 2

int main(int argc, char** argv) {

    Support::Timer t_total;
    t_total.start();
    MyFlatZincOptions opt("Gecode/MyFlatZinc");
    opt.parse(argc, argv);


    //std::cerr << opt.solutions() << std::endl;

    MyFlatZincSpace* fg = NULL;

    FlatZinc::Printer p;
    FlatZinc::Printer p1;

    switch(opt.model()) {
    case MyFlatZincOptions::MODEL_FLATZINC: {
        if (argc!=2) {
            cerr << "Usage: " << argv[0] << " [options] <file>" << endl;
            cerr << "       " << argv[0] << " -help for more information" << endl;
            exit(EXIT_FAILURE);
        }
        const char* filename = argv[1];
        opt.name(filename);

        MyFlatZincSpace* fg_copy = NULL;
        if (!strcmp(filename, "-")) {
            fg = new MyFlatZincSpace(); //mandatory
            fg = static_cast<MyFlatZincSpace*>(FlatZinc::parse(cin, p, cerr, fg));

            if(opt.search() != MyFlatZincOptions::FZ_SEARCH_BAB) {
                fg_copy = new MyFlatZincSpace();
                fg_copy = static_cast<MyFlatZincSpace*>(FlatZinc::parse(cin, p1, cerr, fg_copy));
            }
        } else {
            fg = new MyFlatZincSpace(); //mandatory
            fg = static_cast<MyFlatZincSpace*>(FlatZinc::parse(filename, p, cerr, fg));

            if(opt.search() != MyFlatZincOptions::FZ_SEARCH_BAB) {
                fg_copy = new MyFlatZincSpace();
                fg_copy = static_cast<MyFlatZincSpace*>(FlatZinc::parse(filename, p1, cerr, fg_copy));
            }
        }
        delete fg_copy;
    }
    break;

    case MyFlatZincOptions::MODEL_NQUEENS:

        fg = Queens::getInstance(opt);
        break;

    case MyFlatZincOptions::MODEL_GOLOMBRULER:

        fg = GolombRuler::getInstance(opt);
        break;

    case MyFlatZincOptions::MODEL_ALLINTERVAL:

        fg = AllInterval::getInstance(opt);
        break;

    case MyFlatZincOptions::MODEL_LATINSQUARES:

        fg = LatinSquares::getInstance(opt);
        break;

    case MyFlatZincOptions::MODEL_MAGICSQUARE:

        fg = MagicSquare::getInstance(opt);
        break;

    case MyFlatZincOptions::MODEL_PARTITION:

        fg = Partition::getInstance(opt);
        break;

    case MyFlatZincOptions::MODEL_BACP:

        fg = BACP::getInstance(opt);
        break;

    case MyFlatZincOptions::MODEL_GRAPHCOLOR:

        fg = GraphColor::getInstance(opt);
        break;

    case MyFlatZincOptions::MODEL_BINPACKING: {
        if (argc!=2) {
            cerr << "Usage: " << argv[0] << " [options] <file>" << endl;
            cerr << "       " << argv[0] << " -help for more information" << endl;
            exit(EXIT_FAILURE);
        }
        const char* filename = argv[1];
        string name_instance("binpacking_" + string(filename));
        opt.name(name_instance.c_str());

        fg = BinPacking::getInstance(opt, filename);
    }
    break;

    case MyFlatZincOptions::MODEL_SPORTSLEAGUE:

        if (opt.nsize() != 0 && opt.nsize() % 2 != 0) {
            cerr << "Sports League must have a non zero and pair number of teams\n";
            exit(EXIT_FAILURE);
        }
        fg = SportsLeague::getInstance(opt);
        break;

    case MyFlatZincOptions::MODEL_QUASIGROUPSCOMPLETION:

        fg = QuasiGroupsCompletion::getInstance(opt);
        break;

    case MyFlatZincOptions::MODEL_STEELMILL: {
        char* filename = NULL;

        if (argc >= 2) {
            filename = argv[1];
            string name_instance("steelmill_" + getBaseName(filename));
            opt.name(name_instance.c_str());

        } else {
            string name_instance("steelmill_example");
            opt.name(name_instance.c_str());

        }

        fg = SteelMill::getInstance(opt, filename);

    }
    break;

    case MyFlatZincOptions::MODEL_ENUMERATION:

        fg = Enumeration::getInstance(opt);
        break;


    default:
        cerr << "Unknown Model !!!\n";
        exit(-1);
    }

    if (fg) {
        if(opt.add_ub()) {
            if(fg->method() != MyFlatZincSpace::SAT) {
#ifdef _DEBUG
                std::cerr << "set upperbound : " << opt.ub() << endl;
#endif
                if (fg->method() == MyFlatZincSpace::MIN) {
                    Gecode::rel(*fg, fg->iv[fg->optVar()], Gecode::IRT_LQ, opt.ub());
                } else {
                    Gecode::rel(*fg, fg->iv[fg->optVar()], Gecode::IRT_GQ, opt.ub());
                }
            }
        }

        if(opt.add_lb()) {
            if(fg->method() != MyFlatZincSpace::SAT) {
#ifdef _DEBUG
                std::cerr << "set lowerbound : " << opt.lb() << endl;
#endif
                if (fg->method() == MyFlatZincSpace::MIN) {
                    Gecode::rel(*fg, fg->iv[fg->optVar()], Gecode::IRT_GQ, opt.lb());
                } else {
                    Gecode::rel(*fg, fg->iv[fg->optVar()], Gecode::IRT_LQ, opt.lb());
                }
            }
        }



        if(opt.search() == MyFlatZincOptions::FZ_SEARCH_BAB) {
            //Use BranchFilter to remove variable in branching
            //::memcpy(&p1, &p, sizeof(FlatZinc::Printer)); <!-- does not work

            if(opt.model() == MyFlatZincOptions::MODEL_FLATZINC) {
                fg->createBranchers(fg->solveAnnotations(), opt.seed(), opt.decay(), false, std::cerr);

                //branch(*fg, fg->iv, TieBreak<IntVarBranch>(INT_VAR_NONE()), Gecode::INT_VAL_MIN());
                //branch(*fg, fg->bv, TieBreak<IntVarBranch>(INT_VAR_NONE()), Gecode::INT_VAL_MIN());

                fg->shrinkArrays(p);

            } else {

                fg->createBranchers();
            }

            fg->_space_hook = NULL;

        } else {

            //Space stable
            //
            //fg->sortVariables(true);
            //StatusStatistics sstat;

            if (fg->status() == SS_FAILED) {
                std::cerr << "SPACE FAILED" << endl;
            }

            //Problem d'objective il faut mettre <= 21 pas failed !!!!!!
            fg->_space_hook = static_cast<MyFlatZincSpace*>(fg->clone(false));

            if(opt.model() == MyFlatZincOptions::MODEL_FLATZINC) {
                fg->createBranchers(fg->solveAnnotations(), opt.seed(), opt.decay(), false, std::cerr);
                //branch(*fg, fg->iv, TieBreak<IntVarBranch>(INT_VAR_NONE()), Gecode::INT_VAL_MIN());
                //branch(*fg, fg->bv, TieBreak<IntVarBranch>(INT_VAR_NONE()), Gecode::INT_VAL_MIN());

                //std::cerr << "number of variables int model hook : " << fg->_space_hook->iv.size() << std::endl;
                fg->shrinkArrays(p);
                fg->_space_hook->shrinkArrays(p1);
                //std::cerr << "number of variables int model hook : " << fg->_space_hook->iv.size() << std::endl;
                //getchar();

                //true
                fg->sortVariables(true);
                fg->_space_hook->sortVariables(true);

                //fg->status();
                //fg->_space_hook->status();

            } else {

                fg->createBranchers();

            }

            //std::cerr << "number of variables int models : " << fg->iv.size() << std::endl;
            //std::cerr << "number of variables bool models : " << fg->bv.size() << std::endl;
            //getchar();
        }
        //MySearchOptions opt2;
        //MasterProcess(fg, opt2, 1);

        //EPS_GRID_COMPUTATION
        //
        if(opt.search() == MyFlatZincOptions::FZ_SEARCH_EPS_GRID_COMPUTATION) {

            ifstream fin(opt.sp_file());
            if (!fin.good()) {
                std::cerr << "Could not open file " << opt.sp_file() << " for reading subproblem."
                          << std::endl;
                exit(EXIT_FAILURE);
            }

            readSubProblem(fg, fin);

            if(fg->method() != MyFlatZincSpace::SAT) {
                //Read Objective File
                MyLockFile lockF(opt.obj_file());
                lockF.lock();
                char data[21]; //=> 2^64 = 18446744073709551616 (20 figures)
                int rd = lockF.read(data, 21);
                if(rd) {
                    data[rd] = '\0';
                    int currentObj = ::atoi(data);
                    if (fg->method() == MyFlatZincSpace::MIN) {
                        Gecode::rel(*fg, fg->iv[fg->optVar()], Gecode::IRT_LQ, currentObj);
                        if(fg->_space_hook) {
                            Gecode::rel(*fg->_space_hook, fg->_space_hook->iv[fg->_space_hook->optVar()], Gecode::IRT_LQ, currentObj);
                        }
                    } else {
                        Gecode::rel(*fg, fg->iv[fg->optVar()], Gecode::IRT_GQ, currentObj);
                        if(fg->_space_hook) {
                            Gecode::rel(*fg->_space_hook, fg->_space_hook->iv[fg->_space_hook->optVar()], Gecode::IRT_GQ, currentObj);
                        }
                    }
                } else {
                    std::cerr << "No objective value found in file " << opt.obj_file() << std::endl;
                }
                lockF.unlock();
            }
        }

        //Set status to space_hook
        if (fg->_space_hook && fg->_space_hook->status() == SS_FAILED) {
            std::cerr << "SPACE HOOK FAILED" << endl;
        }

        /*
        std::cerr << "decision variables : " << fg->_space_hook->iv.size() << std::endl;
        cerr << "indexVar Obj : " << fg->_space_hook->optVar() << endl;
        for(int i = 0; i < fg->_space_hook->iv.size(); i++) {
            cerr << "[" << fg->_space_hook->iv[i].min() << "..." << fg->_space_hook->iv[i].max() << "], ";
        }
        cerr << endl;


        std::cerr << "decision variables : " << fg->iv.size() << std::endl;
        cerr << "indexVar Obj : " << fg->optVar() << endl;
        for(int i = 0; i < fg->iv.size(); i++) {
            cerr << "[" << fg->iv[i].min() << "..." << fg->iv[i].max() << "], ";
        }
        cerr << endl;
        getchar();
        */


        if (opt.output()) {
            std::ofstream os(opt.output());
            if (!os.good()) {
                std::cerr << "Could not open file " << opt.output() << " for output."
                          << std::endl;
                exit(EXIT_FAILURE);
            }

            fg->run(os, getBaseName(opt.name()), p, opt, t_total);
            os.close();
        } else {
            fg->run(std::cout, getBaseName(opt.name()), p, opt, t_total);
        }

    } else {
        exit(EXIT_FAILURE);
    }

    delete fg;

    //getchar();
    return 0;
}


string getBaseName(const string& path) {

    string separator_directory = "/";
    if(path.find("\\") != string::npos)
        separator_directory = "\\";

    string filename(path.substr(path.find_last_of(separator_directory) + 1, path.size()));

    //if there is no directory, give the filename
    if(filename == "" && path.find(separator_directory) == string::npos)
        filename = path;

    return filename;
}

#define TOKEN_RESULT_SOL "s"
#define TOKEN_RESULT_OBJ "o"
#define TOKEN_RESULT_TIME "T"
#define TOKEN_RESULT_NODE "n"
#define TOKEN_RESULT_FAIL "f"
#define TOKEN_RESULT_DEPTH "d"
#define TOKEN_RESULT_PROP "p"

#define	TOKEN_VAR "v"
#define	TOKEN_TUPLE "t"
#define	TOKEN_UB "ub"
#define	TOKEN_LB "lb"

void readSubProblem(MyFlatZincSpace* fg, istream& fin) {

    string line;
    string type;
    int num;

    Gecode::BoolVarArgs vars_bool;
    Gecode::IntVarArgs vars_int;

    Gecode::BoolVarArgs vars_hook_bool;
    Gecode::IntVarArgs vars_hook_int;

    Gecode::TupleSet* tupleSet = NULL;
    while (!getline(fin, line).eof()) {

        istringstream ss(line);
        ss >> type;

        if(type == TOKEN_VAR) {

            if(tupleSet) {
                tupleSet->finalize();
#ifdef _DEBUG
                /*
                std::cerr << "BEGIN TUPLE" << std::endl;
                for(int i = 0; i < tupleSet->tuples(); i++) {
                    for(int j = 0; j < tupleSet->arity(); j++) {
                        std::cerr << (*tupleSet)[i][j] << " ";
                    }
                    std::cerr << std::endl;
                }
                std::cerr << "END TUPLE" << std::endl;
                */
#endif

                if(vars_bool.size()) {
                    Gecode::extensional(*fg, vars_bool, *tupleSet);
                    if(fg->_space_hook) {
                        Gecode::extensional(*fg->_space_hook, vars_bool, *tupleSet);
                    }
                } else if(vars_int.size()) {
                    Gecode::extensional(*fg, vars_int, *tupleSet);
                    if(fg->_space_hook) {
                        Gecode::extensional(*fg->_space_hook, vars_int, *tupleSet);
                    }
                }

                delete tupleSet;
                tupleSet = new Gecode::TupleSet();

            } else {
                tupleSet = new Gecode::TupleSet();
            }

            vars_bool = Gecode::BoolVarArgs();
            vars_int = Gecode::IntVarArgs();

            if(fg->_space_hook) {
                vars_hook_bool = Gecode::BoolVarArgs();
                vars_hook_int = Gecode::IntVarArgs();
            }

            while(ss >> num) {
                if(num < fg->bv.size()) {
                    vars_bool << fg->bv[num];
                    if(fg->_space_hook) {
                        vars_hook_bool << fg->_space_hook->bv[num];
                    }
                } else {
                    vars_int << fg->iv[num - fg->bv.size()];
                    if(fg->_space_hook) {
                        vars_hook_int << fg->_space_hook->iv[num - fg->_space_hook->bv.size()];
                    }
                }
            }
        } else if(type == TOKEN_TUPLE) {
            Gecode::IntArgs tuple;
            while(ss >> num) {
                tuple << num;
            }
            tupleSet->add(tuple);
        } else if(type == TOKEN_UB) {
            if(ss >> num) {
                if (fg->method() == MyFlatZincSpace::MIN) {
                    Gecode::rel(*fg, fg->iv[fg->optVar()], Gecode::IRT_LQ, num);
                } else if (fg->method() == MyFlatZincSpace::MAX) {
                    Gecode::rel(*fg, fg->iv[fg->optVar()], Gecode::IRT_GQ, num);
                }
            }
        } else if(type == TOKEN_LB) {
            if(ss >> num) {
                if (fg->method() == MyFlatZincSpace::MIN) {
                    Gecode::rel(*fg, fg->iv[fg->optVar()], Gecode::IRT_GQ, num);
                } else if (fg->method() == MyFlatZincSpace::MAX) {
                    Gecode::rel(*fg, fg->iv[fg->optVar()], Gecode::IRT_LQ, num);
                }
            }
        }
    }

    if(tupleSet) {
        tupleSet->finalize();

#ifdef _DEBUG

        /*
        std::cerr << "BEGIN TUPLE" << std::endl;
        for(int i = 0; i < tupleSet->tuples(); i++) {
            for(int j = 0; j < tupleSet->arity(); j++) {
                std::cerr << (*tupleSet)[i][j] << " ";
            }
            std::cerr << std::endl;
        }
        std::cerr << "END TUPLE" << std::endl;
        */
#endif

        if(vars_bool.size()) {
            Gecode::extensional(*fg, vars_bool, *tupleSet);
            if(fg->_space_hook) {
                Gecode::extensional(*fg->_space_hook, vars_hook_bool, *tupleSet);
            }
        } else if(vars_int.size()) {
            Gecode::extensional(*fg, vars_int, *tupleSet);
            if(fg->_space_hook) {
                Gecode::extensional(*fg->_space_hook, vars_hook_int, *tupleSet);
            }
        }
    }

    delete tupleSet;
    tupleSet = NULL;
}
