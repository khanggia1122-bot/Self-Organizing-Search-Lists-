#include <iostream>
#include <string>
#include <cmath>
#include "list.h"
#include "workload.h"

using namespace std;

int passed = 0;
int failed = 0;

void check(bool condition, const string& name) {
    if (condition) {
        passed++;
        cout << "  [PASS] " << name << endl;
    } else {
        failed++;
        cout << "  [FAIL] " << name << endl;
    }
}

bool within(double actual, double expected, double percent) {
    return fabs(actual - expected) <= expected * percent / 100.0;
}

void fillOneToFive(LinearList& list) {
    for (int k = 1; k <= 5; k++) list.insert(k);
}

void testHandTraceable() {
    cout << "=== TEST 1: Small hand-traceable example (list 1 2 3 4 5; queries 4, 4, 5, 2) ===" << endl;
    const int queries[4] = {4, 4, 5, 2};
    StaticList s;
    MoveToFrontList m;
    TransposeList t;
    LinearList* lists[3] = {&s, &m, &t};
    int totals[3] = {0, 0, 0};
    for (int i = 0; i < 3; i++) {
        fillOneToFive(*lists[i]);
        cout << "  " << lists[i]->name() << ": start [" << lists[i]->toString() << "]" << endl;
        for (int j = 0; j < 4; j++) {
            int c;
            lists[i]->search(queries[j], c);
            totals[i] += c;
            cout << "    search " << queries[j] << ": " << c << " comparisons -> [" << lists[i]->toString() << "]" << endl;
        }
        cout << "    total = " << totals[i] << endl;
    }
    check(totals[0] == 15 && s.toString() == "1 2 3 4 5", "Static: total 15, list unchanged (matches hand trace)");
    check(totals[1] == 14 && m.toString() == "2 5 4 1 3", "Move-To-Front: total 14, final list 2 5 4 1 3 (matches hand trace)");
    check(totals[2] == 15 && t.toString() == "1 2 4 5 3", "Transpose: total 15, final list 1 2 4 5 3 (matches hand trace)");
}

void testEdgeCases() {
    cout << "=== TEST 2: Edge cases ===" << endl;
    {
        MoveToFrontList l;
        int c = -1;
        bool found = l.search(7, c);
        check(!found && c == 0 && l.size() == 0, "Empty list: search finds nothing with 0 comparisons");
        check(!l.remove(7), "Empty list: delete returns false");
    }
    {
        TransposeList l;
        l.insert(42);
        int c;
        check(l.search(42, c) && c == 1 && l.toString() == "42", "Single element: found with 1 comparison, list unchanged");
        check(l.remove(42) && l.size() == 0 && l.toString() == "", "Single element: delete empties the list");
    }
    {
        MoveToFrontList l;
        bool firstAccepted = l.insert(7);
        int extraAccepted = 0;
        for (int i = 0; i < 4; i++) if (l.insert(7)) extraAccepted++;
        check(firstAccepted && extraAccepted == 0 && l.size() == 1, "All-duplicate inserts: first is accepted, 4 repeats are rejected, size stays 1");
    }
    {
        TransposeList l;
        fillOneToFive(l);
        check(l.remove(1) && l.toString() == "2 3 4 5" && l.isLinkConsistent(), "Delete head keeps links consistent");
        check(l.remove(5) && l.toString() == "2 3 4" && l.isLinkConsistent(), "Delete tail keeps links consistent");
        check(l.remove(3) && l.toString() == "2 4" && l.isLinkConsistent(), "Delete middle keeps links consistent");
        check(!l.remove(99) && l.size() == 2, "Delete missing key returns false and changes nothing");
        int c;
        check(!l.search(99, c) && c == 2, "Search for a missing key costs the full list length");
    }
}

void testLargeScale() {
    cout << "=== TEST 3: Large-scale skewed workload (N=1000, 80% of queries hit 200 hot items) ===" << endl;
    const int n = 1000;
    const int hot = 200;
    const int q = 400000;
    Rng rng(20261015ULL);
    int* order = new int[n];
    int* queries = new int[q];
    makeShuffledKeys(order, n, rng);
    makeSkewedQueries(queries, q, n, hot, 80, rng);

    StaticList s;
    MoveToFrontList m;
    TransposeList t;
    LinearList* lists[3] = {&s, &m, &t};
    double steady[3];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < n; j++) lists[i]->insert(order[j]);
        long long sum = 0;
        for (int j = 0; j < q; j++) {
            int c;
            lists[i]->search(queries[j], c);
            if (j >= q / 2) sum += c;
        }
        steady[i] = (double)sum / (q - q / 2);
        cout << "  " << lists[i]->name() << ": last-half average = " << steady[i] << endl;
    }
    check(within(steady[0], (n + 1) / 2.0, 2.0), "Static matches (N+1)/2 within 2%");
    check(steady[1] < 0.55 * steady[0], "Move-To-Front is much cheaper than Static on skewed input");
    check(steady[2] < steady[0], "Transpose is cheaper than Static on skewed input");
    check(s.size() == n && m.size() == n && t.size() == n && m.isLinkConsistent() && t.isLinkConsistent(),
          "Lists keep all N keys and consistent links after 400,000 reorganizations");
    delete[] order;
    delete[] queries;
}

void testUniformNoBenefit() {
    cout << "=== TEST 4: Uniform workload gives no advantage ===" << endl;
    const int n = 500;
    const int q = 100000;
    Rng rng(99);
    int* order = new int[n];
    int* queries = new int[q];
    makeShuffledKeys(order, n, rng);
    makeUniformQueries(queries, q, n, rng);
    StaticList s;
    MoveToFrontList m;
    TransposeList t;
    LinearList* lists[3] = {&s, &m, &t};
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < n; j++) lists[i]->insert(order[j]);
        for (int j = 0; j < q; j++) {
            int c;
            lists[i]->search(queries[j], c);
        }
        check(within(lists[i]->averageComparisons(), (n + 1) / 2.0, 3.0), lists[i]->name() + " stays near (N+1)/2 when there is no skew");
    }
    delete[] order;
    delete[] queries;
}

void testStaticBaseline() {
    cout << "=== TEST 5: Static baseline counts comparisons and never reorders ===" << endl;
    StaticList l;
    fillOneToFive(l);
    int c;
    check(l.search(4, c) && c == 4, "Search 4 in 1 2 3 4 5 costs 4 comparisons");
    check(l.toString() == "1 2 3 4 5", "List is unchanged after a successful search");
    check(l.search(1, c) && c == 1, "Search for the head costs 1 comparison");
    check(!l.search(9, c) && c == 5, "Search for a missing key costs 5 comparisons");
    check(l.getTotalSearches() == 3 && l.getTotalComparisons() == 10, "Statistics: 3 searches, 10 comparisons in total");
    check(l.averageComparisons() > 3.33 && l.averageComparisons() < 3.34, "Average comparisons per search = 10 / 3");
}

void testWorkloadGenerator() {
    cout << "=== TEST 6: Workload generator ===" << endl;
    const int n = 1000;
    const int hot = 200;
    const int q = 100000;
    Rng rng(5);
    int* keys = new int[n];
    makeShuffledKeys(keys, n, rng);
    int* seen = new int[n];
    for (int i = 0; i < n; i++) seen[i] = 0;
    bool inRange = true;
    for (int i = 0; i < n; i++) {
        if (keys[i] < 0 || keys[i] >= n) inRange = false;
        else seen[keys[i]]++;
    }
    bool permutation = inRange;
    for (int i = 0; i < n; i++) if (seen[i] != 1) permutation = false;
    check(permutation, "Shuffled keys are a permutation of 0..N-1");
    check(keys[0] != 0 || keys[1] != 1 || keys[2] != 2, "Shuffled keys are not left in sorted order");

    int* queries = new int[q];
    makeSkewedQueries(queries, q, n, hot, 80, rng);
    int hotHits = 0;
    bool valid = true;
    for (int i = 0; i < q; i++) {
        if (queries[i] < 0 || queries[i] >= n) valid = false;
        if (queries[i] < hot) hotHits++;
    }
    check(valid, "Every generated query is a valid key");
    check(within((double)hotHits / q * 100.0, 80.0, 1.5), "About 80% of queries hit the 200 hot keys");

    Rng a(123), b(123);
    check(a.next() == b.next() && a.next() == b.next(), "Same seed gives the same sequence");
    delete[] keys;
    delete[] seen;
    delete[] queries;
}

int main() {
    testHandTraceable();
    cout << endl;
    testEdgeCases();
    cout << endl;
    testLargeScale();
    cout << endl;
    testUniformNoBenefit();
    cout << endl;
    testStaticBaseline();
    cout << endl;
    testWorkloadGenerator();
    cout << endl << "Result: " << passed << " passed, " << failed << " failed" << endl;
    return failed == 0 ? 0 : 1;
}
