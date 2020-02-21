#include <memory>
#include <iostream>
#include <deque>
#include <queue>
#include <fstream>
#include <utility>
#include <sstream>
#include <vector>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <list>
#include <time.h>
#include <string>
#include <chrono>

#include "lib/roaring.hh"
#include "lib/roaring.c"
#include "lib/fastrand.c"

using namespace std;
using namespace std::chrono;

#define FOR(i, a, b) for (long i(a), _b(b); i < _b; ++i)

typedef uint32_t U32;

struct Library
{
    U32 booksPerDay;
    U32 timeToSign;
    vector<U32> books;
    U32 score = 0;
};

U32 B;
U32 L;
U32 D;
vector<U32> BOOKS;
vector<Library> LIBRARIES;

void parseInput(istream &istr)
{
    istr >> B;
    istr >> L;
    istr >> D;

    BOOKS.resize(B);
    FOR(i, 0, B)
    {
        istr >> BOOKS[i];
    }

    LIBRARIES.resize(L);
    FOR(i, 0, L)
    {
        auto& lib = LIBRARIES[i];
        istr >> lib.timeToSign >> lib.booksPerDay;

        U32 nbBooks;
        istr >> nbBooks;
        lib.books.resize(nbBooks);

        FOR(b, 0, nbBooks)
        {
            istr >> lib.books[b];
            lib.score += BOOKS[lib.books[b]];
        }
    }
}

using ScoredLibrary = pair<U32, U32>; // second is score
using ScoredBook = pair<U32, U32>; // second is score

struct CompareReverseSecond
{
    bool operator()(const ScoredBook& lhs, const ScoredBook& rhs)
    {
        return rhs.second < lhs.second;
    }
};

struct InactiveLibrary
{
    U32 id;
    vector<U32> booksTaken;
};

struct ActiveLibrary
{
    U32 id;
    vector<U32> booksTaken;

    priority_queue<ScoredBook, vector<ScoredBook>, CompareReverseSecond> bookQueue;

    ActiveLibrary(U32 id): id(id)
    {
        for (U32 book : LIBRARIES[id].books)
        {
            bookQueue.push({ book, BOOKS[book] });
        }
    }
};

struct Solution
{
    using LibrariesToSign = priority_queue<ScoredLibrary, vector<ScoredLibrary>, CompareReverseSecond>;

    vector<InactiveLibrary> librariesSelected;
    U32 score = 0;

    void solve()
    {
        vector<ActiveLibrary> librariesSignedUp;
        Roaring booksTaken;

        optional<ActiveLibrary> libraryToSignup;
        U32 librarySignupDeadline = 0;

        LibrariesToSign librariesToSign;
        // librariesToSign.reserve(L);
        FOR(l, 0, L) {
            librariesToSign.push({ l, LIBRARIES[l].score });
        }

        U32 d = 0;
        while (d < D)
        {
            if (d % 1000 == 0)
                cerr << "day " << d << " / " << D << endl;

            // sign up a library (if possible)
            if (d == librarySignupDeadline)
            {
                if (libraryToSignup)
                {
                    librariesSignedUp.push_back(move(*libraryToSignup));
                    libraryToSignup = nullopt;
                }

                // choose best library to sign up
                while (!librariesToSign.empty())
                {
                    U32 id = takeBestLibrary(librariesToSign);
                    U32 timeToSign = LIBRARIES[id].timeToSign;
                    if (d + timeToSign < D)
                    {
                        libraryToSignup = ActiveLibrary(id);
                        librarySignupDeadline = d + timeToSign;
                        cerr << "signing up " << libraryToSignup->id << " in " << timeToSign << " days" << endl;
                        break;
                    }
                }
            }

            // scan books from the signed up libraries
            vector<U32> toDeactivate;
            FOR(i, 0, librariesSignedUp.size())
            {
                auto& l = librariesSignedUp[i];
                FOR(m, 0, LIBRARIES[l.id].booksPerDay)
                {
                    while (!l.bookQueue.empty())
                    {
                        U32 book = takeBestBook(l);
                        if (booksTaken.addChecked(book))
                        {
                            cerr << "taking book " << book << " from " << l.id << endl;
                            l.booksTaken.push_back(book);
                            score += BOOKS[book];
                            break;
                        }
                    }
                }

                if (l.bookQueue.empty())
                    toDeactivate.push_back(i);
            }

            for (U32 i : toDeactivate)
            {
                auto& l = librariesSignedUp[i];
                librariesSelected.push_back(InactiveLibrary { l.id, move(l.booksTaken) });

                U32 end = librariesSignedUp.size()-1;
                if (i < end) librariesSignedUp[i] = move(librariesSignedUp[end]);
                librariesSignedUp.pop_back();
            }

            U32 daysToElapse = (librariesSignedUp.empty()) ? librarySignupDeadline - d : 1;
            // cerr << "elapsing by " << daysToElapse << " days, " << librariesSignedUp.size() << " libraries active, score = " << score << endl;
            d += daysToElapse;
        }
    }

    void mutate()
    {
    }

    U32 getScore()
    {
        return score;
    }

    void writeToFile(ostream &out)
    {
        out << librariesSelected.size() << endl;
        for (auto &l : librariesSelected)
        {
            out << l.id << " " << l.booksTaken.size() << endl;
            for (auto i : l.booksTaken)
            {
                out << i << " ";
            }
            out << endl;
        }
    }

    U32 takeBestLibrary(LibrariesToSign& librariesToSign)
    {
        ScoredLibrary libraryWithScore = librariesToSign.top();
        librariesToSign.pop();
        return libraryWithScore.first;
    }

    U32 takeBestBook(ActiveLibrary& l)
    {
        ScoredBook bookWithScore = l.bookQueue.top();
        l.bookQueue.pop();
        return bookWithScore.first;
    }
};


int main(int argc, char** argv)
{
    fast_srand((int)time(NULL));

    istream* input = &cin;

    unique_ptr<ifstream> inputFilePtr;
    if (argc > 1)
    {
        cerr << "Reading input from " << argv[1] << endl;
        inputFilePtr.reset(new ifstream(argv[1]));
        input = inputFilePtr.get();
    }

    ostream* output = &cout;
    unique_ptr<ofstream> outputFilePtr;
    if (argc > 2)
    {
        cerr << "Writing output to " << argv[2] << endl;
        outputFilePtr.reset(new ofstream(argv[2]));
        output = outputFilePtr.get();
    }

    parseInput(*input);

    auto start = high_resolution_clock::now();
    Solution solution;
    auto elapsed = high_resolution_clock::now() - start;
    cerr << "Init solution took "<< chrono::duration <double, milli>(elapsed).count() <<" ms"<< endl;

    start = high_resolution_clock::now();
    solution.solve();
    elapsed = high_resolution_clock::now() - start;

    cerr << "Solve solution took "<< chrono::duration <double, milli>(elapsed).count() <<" ms"<< endl;

    start = high_resolution_clock::now();
    U32 score = solution.getScore();
    elapsed = high_resolution_clock::now() - start;

    cerr << "Score solution took "<< chrono::duration <double, milli>(elapsed).count() <<" ms"<< endl;

    solution.writeToFile(*output);

    cerr << "ENDED score: " << score << endl;
}
