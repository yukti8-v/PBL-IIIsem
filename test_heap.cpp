
#include <iostream>
#include "include/priority_queue.h"

int main() {
    MaxHeap<int> heap;

    heap.insert(10);
    heap.insert(40);
    heap.insert(20);
    heap.insert(5);
    heap.insert(30);

    std::cout << "Maximum: " << heap.peek() << '\n';

    std::cout << "Extracting: ";

    while (!heap.empty()) {
        std::cout << heap.extractMax() << ' ';
    }

    std::cout << '\n';
    return 0;
}
