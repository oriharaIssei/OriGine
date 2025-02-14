#pragma once

/// stl
#include <cmath>
#include <vector>

template <typename T>
bool checkSorted(const std::vector<T>& v) {
    bool is_sorted = true;
    int temp       = 0;
    for (const auto& e : v) {
        if (temp > e) {
            is_sorted = false;
        }
        temp = e;
    }
    return is_sorted;
}

template <typename T>
void bubbleSort(std::vector<T>& v) {
    bool sorting     = true;
    size_t sortedIdx = 0;
    while (sorting) {
        sorting = false;
        for (size_t i = 0; i < v.size() - 1 - sortedIdx; ++i) {
            if (v[i] > v[i + 1]) {
                swap(v[i], v[i + 1]);
                if (!sorting)
                    sorting = true;
            }
        }
        sortedIdx++;
    }
}

template <typename T>
void selectionSort(std::vector<T>& v) {
    for (size_t i = 0; i < v.size() - 1; ++i) {
        int minTemp   = v[i];
        size_t minIdx = i;
        for (size_t j = i + 1; j < v.size(); ++j) {
            if (minTemp > v[j]) {
                minTemp = v[j];
                minIdx  = j;
            }
        }
        swap(v[i], v[minIdx]);
    }
}

template <typename T>
void shakerSort(std::vector<T>& v) {
    bool sorting     = true;
    size_t sortedIdx = 0;
    while (sorting) {
        for (size_t i = sortedIdx; i < v.size() - 1; ++i) {
            if (v[i] > v[i + 1]) {
                swap(v[i], v[i + 1]);
            }
        }
        sorting = false;
        for (size_t i = v.size() - 2 - sortedIdx; i < v.size(); --i) {
            if (v[i] > v[i + 1]) {
                swap(v[i], v[i + 1]);
                if (!sorting)
                    sorting = true;
            }
        }
        sortedIdx++;
    }
}

template <typename T>
void combSort(std::vector<T>& v) {
    size_t h       = (v.size() * 10) / 13;
    bool is_sorted = false;
    while (!is_sorted) {
        if (h == 1)
            is_sorted = true;
        for (size_t i = 0; i < v.size() - h; ++i) {
            if (v[i] > v[i + h]) {
                swap(v[i], v[i + h]);
                if (is_sorted)
                    is_sorted = false;
            }
        }
        if (h > 1)
            h = (h * 10) / 13;
        if (h == 0)
            h = 1;
    }
}

template <typename T>
void normSort(std::vector<T>& v) {
    for (size_t i = 1; i < v.size(); ++i) {
        for (size_t j = i; j < v.size(); --j) {
            if (v[j - 1] > v[j]) {
                swap(v[j - 1], v[j]);
            }
        }
    }
}

template <typename T>
void shellSort(std::vector<T>& v) {
    size_t h = 1;
    while (1) {
        size_t nexth = 3 * h + 1;
        if (nexth < v.size())
            h = nexth;
        else
            break;
    }
    bool is_sorted = false;
    while (!is_sorted) {
        if (h == 1)
            is_sorted = true;
        for (size_t i = h; i < v.size(); ++i) {
            for (size_t j = i; j >= h; j -= h) {
                if (v[j - h] > v[j]) {
                    swap(v[j - h], v[j]);
                    if (is_sorted)
                        is_sorted = false;
                }
            }
        }
        h = (h - 1) / 3;
        if (h == 0)
            h = 1;
    }
}
