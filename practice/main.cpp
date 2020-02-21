#include <fstream>
#include <iostream>
#include <sstream>

#include <algorithm>
#include <functional>

#include <deque>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#ifdef __linux__
#include <signal.h>
#include <unistd.h>
#endif

#include "lib/roaring.hh"
#include "lib/roaring.c"
#include "lib/fastrand.c"

using namespace std;

// GENERAL

ofstream devnull;

#define error cerr
#define debug cerr

// #define trace cerr
#define trace devnull

using U8  = uint8_t;
using U16 = uint16_t;
using U32 = uint32_t;
using U64 = uint64_t;

#ifdef __linux__

function<void()> onInterrupt = [](){};

void interruptHandler(int)
{
    cerr << "Interrupted ..." << endl;
    onInterrupt();
}

void setInterruptHandler()
{
    struct sigaction action;
    action.sa_handler = interruptHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, NULL);
}

#endif

void init(int seed)
{
#ifdef __linux__
    setInterruptHandler();
#endif
    fast_srand(seed);
    devnull.open("/dev/null", ofstream::out | ofstream::app);
}

void cleanup()
{
    devnull.close();
}

#include "solution.cpp"

int main(int argc, char** argv)
{
    U32 nbIterations = 10000;
    if (argc > 2)
    {
        istringstream ss(argv[1]);
        if (!(ss >> nbIterations))
        {
            error << "expected a number of iterations (positive integer), but got " << argv[1] << endl;
            return EXIT_FAILURE;
        }
    }

    int seed = time(NULL);
    if (argc > 3)
    {
        istringstream ss(argv[2]);
        if (!(ss >> seed))
        {
            error << "expected a seed (any 32-bit integer), but got " << argv[2] << endl;
            return EXIT_FAILURE;
        }
    }

    init(seed);

    solve(cin, cout, nbIterations);

    cleanup();

    return EXIT_SUCCESS;
}
