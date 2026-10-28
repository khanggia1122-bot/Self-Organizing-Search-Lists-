#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "list.h"
#include "workload.h"
#include "theory.h"

using namespace std;

const int N = 1000;
const int HOT_COUNT = 200;
const int HOT_PERCENT = 80;
const int QUERIES = 200000;
const int BLOCK = 1000;
const int BLOCKS = QUERIES / BLOCK;
const int LONG_QUERIES = 2000000;
const int LONG_BLOCK = 20000;
const int LONG_BLOCKS = LONG_QUERIES / LONG_BLOCK;

struct RunResult {
    double overall;
    double steady;
};

RunResult run(LinearList& list, const int* order, const int* queries, int q, int block, double* blockAvg) {
    for (int i = 0; i < N; i++) list.insert(order[i]);
    long long total = 0;
    long long steady = 0;
    long long blockSum = 0;
    for (int i = 0; i < q; i++) {
        int c;
        list.search(queries[i], c);
        total += c;
        blockSum += c;
        if (i >= q / 2) steady += c;
        if ((i + 1) % block == 0) {
            if (blockAvg != 0) blockAvg[(i + 1) / block - 1] = (double)blockSum / block;
            blockSum = 0;
        }
    }
    RunResult r;
    r.overall = (double)total / q;
    r.steady = (double)steady / (q - q / 2);
    return r;
}

void printRow(const string& name, const RunResult& r) {
    cout << "  " << left << setw(14) << name << right << fixed << setprecision(2)
         << setw(12) << r.overall << setw(14) << r.steady << endl;
}

void runScenario(const string& title, const int* order, const int* queries, int q, int block, double** curves) {
    cout << title << endl;
    cout << "  " << left << setw(14) << "Strategy" << right << setw(12) << "Overall" << setw(14) << "Last half" << endl;
    StaticList s;
    MoveToFrontList m;
    TransposeList t;
    printRow(s.name(), run(s, order, queries, q, block, curves ? curves[0] : 0));
    printRow(m.name(), run(m, order, queries, q, block, curves ? curves[1] : 0));
    printRow(t.name(), run(t, order, queries, q, block, curves ? curves[2] : 0));
}

int main() {
    Rng rng(20261015ULL);
    int* order = new int[N];
    int* queries = new int[QUERIES];
    makeShuffledKeys(order, N, rng);

    double eStatic = theoryStaticRandomOrder(N);
    double eOpt = theoryOptimalStatic(N, HOT_COUNT, HOT_PERCENT);
    double eMtf = theoryMoveToFront(N, HOT_COUNT, HOT_PERCENT);

    cout << "N = " << N << ", hot items = " << HOT_COUNT << ", hot traffic = " << HOT_PERCENT
         << "%, queries = " << QUERIES << endl << endl;
    cout << fixed << setprecision(2);
    cout << "Theory (expected comparisons per search):" << endl;
    cout << "  Static, random order     : " << eStatic << endl;
    cout << "  Optimal static (sorted)  : " << eOpt << endl;
    cout << "  Move-To-Front (Rivest)   : " << eMtf << endl;
    cout << "  Transpose                : between optimal and Move-To-Front (Rivest, 1976)" << endl << endl;

    double** curves = new double*[3];
    for (int i = 0; i < 3; i++) curves[i] = new double[BLOCKS];

    makeSkewedQueries(queries, QUERIES, N, HOT_COUNT, HOT_PERCENT, rng);
    runScenario("Scenario 1: skewed 80/20 workload", order, queries, QUERIES, BLOCK, curves);

    ofstream csv("cost_over_time.csv");
    csv << "searches,static,move_to_front,transpose,theory_static,theory_optimal,theory_mtf" << endl;
    for (int b = 0; b < BLOCKS; b++) {
        csv << (b + 1) * BLOCK << "," << curves[0][b] << "," << curves[1][b] << "," << curves[2][b]
            << "," << eStatic << "," << eOpt << "," << eMtf << endl;
    }
    csv.close();
    cout << "  -> wrote cost_over_time.csv" << endl << endl;

    makeUniformQueries(queries, QUERIES, N, rng);
    runScenario("Scenario 2: uniform workload (no skew)", order, queries, QUERIES, BLOCK, 0);
    cout << "  theory for all strategies: " << eStatic << endl << endl;

    makeCyclicQueries(queries, QUERIES, order, N);
    runScenario("Scenario 3: adversarial cyclic access (1,2,...,N,1,2,...)", order, queries, QUERIES, BLOCK, 0);
    cout << "  Move-To-Front theory: about " << N << " per search; static: " << eStatic << endl;

    int* longQueries = new int[LONG_QUERIES];
    makeSkewedQueries(longQueries, LONG_QUERIES, N, HOT_COUNT, HOT_PERCENT, rng);
    double** longCurves = new double*[3];
    for (int i = 0; i < 3; i++) longCurves[i] = new double[LONG_BLOCKS];
    cout << endl;
    runScenario("Scenario 4: long run, skewed 80/20, 2,000,000 queries", order, longQueries, LONG_QUERIES, LONG_BLOCK, longCurves);
    ofstream longCsv("cost_long_run.csv");
    longCsv << "searches,static,move_to_front,transpose,theory_static,theory_optimal,theory_mtf" << endl;
    for (int b = 0; b < LONG_BLOCKS; b++) {
        longCsv << (b + 1) * LONG_BLOCK << "," << longCurves[0][b] << "," << longCurves[1][b] << "," << longCurves[2][b]
                << "," << eStatic << "," << eOpt << "," << eMtf << endl;
    }
    longCsv.close();
    cout << "  -> wrote cost_long_run.csv" << endl;

    for (int i = 0; i < 3; i++) {
        delete[] curves[i];
        delete[] longCurves[i];
    }
    delete[] curves;
    delete[] longCurves;
    delete[] longQueries;
    delete[] queries;
    delete[] order;
    return 0;
}
