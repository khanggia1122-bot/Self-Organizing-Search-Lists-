#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "list.h"
#include "workload.h"

using namespace std;

const int N = 1000;
const int HOT_COUNT = 200;
const int HOT_PERCENT = 80;
const int QUERIES = 200000;
const int BLOCK = 1000;
const int BLOCKS = QUERIES / BLOCK;

struct RunResult {
    double overall;
    double steady;
};

RunResult run(LinearList& list, const int* order, const int* queries, int q, double* blockAvg) {
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
        if ((i + 1) % BLOCK == 0) {
            if (blockAvg != 0) blockAvg[(i + 1) / BLOCK - 1] = (double)blockSum / BLOCK;
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

void runScenario(const string& title, const int* order, const int* queries, int q, double** curves) {
    cout << title << endl;
    cout << "  " << left << setw(14) << "Strategy" << right << setw(12) << "Overall" << setw(14) << "Last half" << endl;
    StaticList s;
    MoveToFrontList m;
    TransposeList t;
    printRow(s.name(), run(s, order, queries, q, curves ? curves[0] : 0));
    printRow(m.name(), run(m, order, queries, q, curves ? curves[1] : 0));
    printRow(t.name(), run(t, order, queries, q, curves ? curves[2] : 0));
}

int main() {
    Rng rng(20261015ULL);
    int* order = new int[N];
    int* queries = new int[QUERIES];
    makeShuffledKeys(order, N, rng);

    cout << "N = " << N << ", hot items = " << HOT_COUNT << ", hot traffic = " << HOT_PERCENT
         << "%, queries = " << QUERIES << endl << endl;

    double** curves = new double*[3];
    for (int i = 0; i < 3; i++) curves[i] = new double[BLOCKS];

    makeSkewedQueries(queries, QUERIES, N, HOT_COUNT, HOT_PERCENT, rng);
    runScenario("Scenario 1: skewed 80/20 workload", order, queries, QUERIES, curves);

    ofstream csv("cost_over_time.csv");
    csv << "searches,static,move_to_front,transpose" << endl;
    for (int b = 0; b < BLOCKS; b++) {
        csv << (b + 1) * BLOCK << "," << curves[0][b] << "," << curves[1][b] << "," << curves[2][b] << endl;
    }
    csv.close();
    cout << "  -> wrote cost_over_time.csv" << endl << endl;

    makeUniformQueries(queries, QUERIES, N, rng);
    runScenario("Scenario 2: uniform workload (no skew)", order, queries, QUERIES, 0);

    for (int i = 0; i < 3; i++) delete[] curves[i];
    delete[] curves;
    delete[] queries;
    delete[] order;
    return 0;
}
