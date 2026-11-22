
#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include <vector>
#include <stdexcept>
#include <utility>
#include <cstddef>

template <typename T>
class MaxHeap {
private:
    std::vector<T> heap;

    void siftUp(std::size_t i) {
        while (i > 0) {
            std::size_t parent = (i - 1) / 2;

            if (!(heap[parent] < heap[i]))
                break;

            std::swap(heap[parent], heap[i]);
            i = parent;
        }
    }

    void siftDown(std::size_t i) {
        while (true) {
            std::size_t left = 2 * i + 1;
            std::size_t right = 2 * i + 2;
            std::size_t largest = i;

            if (left < heap.size() &&
                heap[largest] < heap[left])
                largest = left;

            if (right < heap.size() &&
                heap[largest] < heap[right])
                largest = right;

            if (largest == i)
                break;

            std::swap(heap[i], heap[largest]);
            i = largest;
        }
    }

public:
    bool empty() const {
        return heap.empty();
    }

    std::size_t size() const {
        return heap.size();
    }

    void insert(const T& value) {
        heap.push_back(value);
        siftUp(heap.size() - 1);
    }

    const T& peek() const {
        if (empty())
            throw std::out_of_range("Heap is empty");

        return heap.front();
    }

    T extractMax() {
        if (empty())
            throw std::out_of_range("Heap is empty");

        T result = heap.front();

        if (heap.size() == 1) {
            heap.pop_back();
            return result;
        }

        heap.front() = heap.back();
        heap.pop_back();
        siftDown(0);

        return result;
    }
};

#endif
