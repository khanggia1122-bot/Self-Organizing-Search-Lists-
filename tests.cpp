#include <iostream>
#include <string>
#include "list.h"

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

void fillOneToFive(LinearList& list) {
    for (int k = 1; k <= 5; k++) list.insert(k);
}

void testEdgeCases() {
    cout << "=== TEST 2: Edge cases ===" << endl;
    {
        StaticList l;
        int c = -1;
        bool found = l.search(7, c);
        check(!found && c == 0 && l.size() == 0, "Empty list: search finds nothing with 0 comparisons");
        check(!l.remove(7), "Empty list: delete returns false");
    }
    {
        StaticList l;
        l.insert(42);
        int c;
        check(l.search(42, c) && c == 1 && l.toString() == "42", "Single element: found with 1 comparison, list unchanged");
        check(l.remove(42) && l.size() == 0 && l.toString() == "", "Single element: delete empties the list");
    }
    {
        StaticList l;
        bool firstAccepted = l.insert(7);
        int extraAccepted = 0;
        for (int i = 0; i < 4; i++) if (l.insert(7)) extraAccepted++;
        check(firstAccepted && extraAccepted == 0 && l.size() == 1, "All-duplicate inserts: first is accepted, 4 repeats are rejected, size stays 1");
    }
    {
        StaticList l;
        fillOneToFive(l);
        check(l.remove(1) && l.toString() == "2 3 4 5" && l.isLinkConsistent(), "Delete head keeps links consistent");
        check(l.remove(5) && l.toString() == "2 3 4" && l.isLinkConsistent(), "Delete tail keeps links consistent");
        check(l.remove(3) && l.toString() == "2 4" && l.isLinkConsistent(), "Delete middle keeps links consistent");
        check(!l.remove(99) && l.size() == 2, "Delete missing key returns false and changes nothing");
        int c;
        check(!l.search(99, c) && c == 2, "Search for a missing key costs the full list length");
    }
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

int main() {
    testEdgeCases();
    cout << endl;
    testStaticBaseline();
    cout << endl << "Result: " << passed << " passed, " << failed << " failed" << endl;
    return failed == 0 ? 0 : 1;
}
