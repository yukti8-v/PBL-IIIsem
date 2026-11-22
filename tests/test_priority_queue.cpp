
#include <iostream>
#include <cassert>
#include <stdexcept>
#include "priority_queue.h"

using namespace std;

int main() {
    MaxHeap<int> heap;

    // Test 1: Empty heap
    assert(heap.empty());
    assert(heap.size() == 0);

    // Test 2: Insert elements
    heap.insert(10);
    heap.insert(40);
    heap.insert(20);
    heap.insert(5);
    heap.insert(30);

    assert(heap.size() == 5);
    assert(heap.peek() == 40);

    // Test 3: Extract in descending order
    int expected[] = {40, 30, 20, 10, 5};

    for (int value : expected) {
        assert(!heap.empty());
        assert(heap.extractMax() == value);
    }

    assert(heap.empty());

    // Test 4: Duplicate values
    heap.insert(20);
    heap.insert(20);
    heap.insert(10);

    assert(heap.extractMax() == 20);
    assert(heap.extractMax() == 20);
    assert(heap.extractMax() == 10);

    // Test 5: Negative values
    heap.insert(-5);
    heap.insert(-20);
    heap.insert(-1);

    assert(heap.extractMax() == -1);
    assert(heap.extractMax() == -5);
    assert(heap.extractMax() == -20);

    // Test 6: Single element
    heap.insert(100);
    assert(heap.peek() == 100);
    assert(heap.extractMax() == 100);
    assert(heap.empty());

    // Test 7: Exception when peeking empty heap
    bool peekException = false;

    try {
        heap.peek();
    }
    catch (const out_of_range&) {
        peekException = true;
    }

    assert(peekException);

    // Test 8: Exception when extracting from empty heap
    bool extractException = false;

    try {
        heap.extractMax();
    }
    catch (const out_of_range&) {
        extractException = true;
    }

    assert(extractException);

    cout << "Priority Queue Tests: 8" << endl;
    cout << "ALL TESTS PASSED" << endl;

    return 0;
}
