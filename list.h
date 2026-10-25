#ifndef LIST_H
#define LIST_H

#include <string>

using namespace std;

struct Node {
    int key;
    Node* prev;
    Node* next;
    Node(int k) : key(k), prev(0), next(0) {}
};

class LinearList {
protected:
    Node* head;
    Node* tail;
    int count;
    long long totalComparisons;
    long long totalSearches;

    virtual void onHit(Node* node) = 0;

    Node* findNode(int key, int& comparisons) const {
        comparisons = 0;
        Node* cur = head;
        while (cur != 0) {
            comparisons++;
            if (cur->key == key) return cur;
            cur = cur->next;
        }
        return 0;
    }

    void unlink(Node* node) {
        if (node->prev != 0) node->prev->next = node->next;
        else head = node->next;
        if (node->next != 0) node->next->prev = node->prev;
        else tail = node->prev;
        node->prev = 0;
        node->next = 0;
    }

    void pushFront(Node* node) {
        node->prev = 0;
        node->next = head;
        if (head != 0) head->prev = node;
        else tail = node;
        head = node;
    }

public:
    LinearList() : head(0), tail(0), count(0), totalComparisons(0), totalSearches(0) {}
    LinearList(const LinearList&) = delete;
    LinearList& operator=(const LinearList&) = delete;
    virtual ~LinearList() {
        Node* cur = head;
        while (cur != 0) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
    }

    virtual string name() const = 0;

    bool search(int key, int& comparisons) {
        Node* node = findNode(key, comparisons);
        totalComparisons += comparisons;
        totalSearches++;
        if (node == 0) return false;
        onHit(node);
        return true;
    }

    bool insert(int key) {
        int c;
        if (findNode(key, c) != 0) return false;
        Node* node = new Node(key);
        node->prev = tail;
        if (tail != 0) tail->next = node;
        else head = node;
        tail = node;
        count++;
        return true;
    }

    bool remove(int key) {
        int c;
        Node* node = findNode(key, c);
        if (node == 0) return false;
        unlink(node);
        delete node;
        count--;
        return true;
    }

    int size() const { return count; }
    long long getTotalComparisons() const { return totalComparisons; }
    long long getTotalSearches() const { return totalSearches; }
    double averageComparisons() const {
        return totalSearches == 0 ? 0.0 : (double)totalComparisons / totalSearches;
    }
    void resetStats() {
        totalComparisons = 0;
        totalSearches = 0;
    }

    string toString() const {
        string s;
        for (Node* cur = head; cur != 0; cur = cur->next) {
            if (!s.empty()) s += " ";
            s += to_string(cur->key);
        }
        return s;
    }

    bool isLinkConsistent() const {
        int n = 0;
        Node* last = 0;
        for (Node* cur = head; cur != 0; cur = cur->next) {
            if (cur->prev != last) return false;
            last = cur;
            n++;
        }
        return last == tail && n == count;
    }
};

class StaticList : public LinearList {
protected:
    void onHit(Node*) {}
public:
    string name() const { return "Static"; }
};

class MoveToFrontList : public LinearList {
protected:
    void onHit(Node* node) {
        if (node == head) return;
        unlink(node);
        pushFront(node);
    }
public:
    string name() const { return "MoveToFront"; }
};

class TransposeList : public LinearList {
protected:
    void onHit(Node* node) {
        Node* p = node->prev;
        if (p == 0) return;
        Node* pp = p->prev;
        Node* nn = node->next;
        if (pp != 0) pp->next = node;
        else head = node;
        node->prev = pp;
        node->next = p;
        p->prev = node;
        p->next = nn;
        if (nn != 0) nn->prev = p;
        else tail = p;
    }
public:
    string name() const { return "Transpose"; }
};

#endif
