using PizzaT = uint32_t;

U32 nbMaxPizzaSlices;
U32 nbPizzas;
vector<int> pizzaSizes;

void parseInput(istream& in)
{
    in >> nbMaxPizzaSlices >> nbPizzas;
    pizzaSizes.resize(nbPizzas);
    for (PizzaT pizza = 0; pizza < nbPizzas; pizza++)
        in >> pizzaSizes[pizza];
    debug << "Max slices: " << nbMaxPizzaSlices << ", pizzas: " << nbPizzas << endl;
}

PizzaT estimateMaxScore()
{
    return nbMaxPizzaSlices;
}

class Solution
{
    PizzaT nbSlices = 0;
    Roaring pizzaSet;
    Roaring availablePizzaSet;

public:
    Solution()
    {
        availablePizzaSet.addRange(0, nbPizzas);
    }

    PizzaT computeScore()
    {
        return nbSlices;
    }

    void writeOutput(ostream& out) const
    {
        out << pizzaSet.cardinality() << endl;
        for (PizzaT p : pizzaSet)
            out << p << ' ';
        out << endl;
    }

    void greed()
    {
        trace << "Solving" << endl;
        while (!availablePizzaSet.isEmpty())
        {
            if (!tryAddPizza(availablePizzaSet.minimum()))
                break;
        }
    }

    void mutate(U32 nbMaxToAdd, U32 nbMaxToRemove)
    {
        trace << "Mutating" << endl;
        U32 nbToRemove = fast_rand() % nbMaxToRemove;
        trace << "Removing " << nbToRemove << " pizzas" << endl;
        for (U32 i = 0; i < nbToRemove; i++)
            removeRandomPizza();

        U32 nbToAdd = fast_rand() % nbMaxToAdd;
        trace << "Adding " << nbToAdd << " pizzas" << endl;
        for (U32 i = 0; i < nbToAdd; i++)
            addRandomPizza();
    }

    void removeRandomPizza()
    {
        if (!pizzaSet.isEmpty())
        {
            PizzaT pizza;
            pizzaSet.select(fast_rand() % pizzaSet.cardinality(), &pizza);
            pizzaSet.remove(pizza);
            availablePizzaSet.add(pizza);
            nbSlices -= pizzaSizes[pizza];
        }
        else
        {
            trace << "No pizzas were removed as none were added" << endl;
        }
    }

    void addRandomPizza()
    {
        if (!availablePizzaSet.isEmpty())
        {
            PizzaT pizza;
            availablePizzaSet.select(fast_rand() % availablePizzaSet.cardinality(), &pizza);
            tryAddPizza(pizza);
        }
        else
        {
            trace << "No pizzas were added as none were available" << endl;
        }
    }

    bool tryAddPizza(PizzaT pizza)
    {
        if (pizzaSizes[pizza] + nbSlices <= nbMaxPizzaSlices)
        {
            if (pizzaSet.addChecked(pizza))
            {
                availablePizzaSet.remove(pizza);
                nbSlices += pizzaSizes[pizza];
                return true;
            }
            else
            {
                trace << "pizza " << pizza << " with " << pizzaSizes[pizza] << " slices not added (duplicate)" << endl;
            }
        }
        else
        {
            trace << "pizza " << pizza << " with " << pizzaSizes[pizza] << " slices not added (too many slices)" << endl;
        }
        return false;
    }
};

static void solve(istream& in, ostream& out, U32 nbIterations)
{
    parseInput(in); // MUST BE FIRST CALL

    Solution bestSolution;

#ifdef __linux__
    onInterrupt = [&bestSolution]() {
        bestSolution.writeOutput(cout);
        cout.flush();
        exit(1);
    };
#endif

    const PizzaT maxScore = estimateMaxScore();
    debug << "Max possible score: " << maxScore << endl;

    bestSolution.greed();
    PizzaT bestScore = bestSolution.computeScore();

    debug << "Greedy: " << bestScore
          << ", distance from max: " << maxScore - bestScore
          << endl;

    const U32 nbMaxToAdd = 500;
    const U32 nbMaxToRemove = 500;
    debug << "Running mutations for " << nbIterations << " iterations"
          << ", mutation ~ +" << nbMaxToAdd << "/-" << nbMaxToRemove << endl;

    for (U32 i = 0; i < nbIterations; i++)
    {
        if (i % 100 == 0)
            debug << i << "/" << nbIterations << endl;

        Solution solution = bestSolution;
        solution.mutate(nbMaxToAdd, nbMaxToRemove);
        solution.greed();

        U32 score = solution.computeScore();
        if (score > bestScore)
        {
            debug << "New best: " << score 
                  << ", distance from max: " << maxScore - bestScore
                  << endl;
            bestSolution = move(solution);
            bestScore = score;
        }

        if (bestScore == maxScore)
        {
            debug << "Max score hit!" << endl;
            break;
        }
    }

    debug << "Final score: " << bestSolution.computeScore() << endl;
    bestSolution.writeOutput(out);
}
