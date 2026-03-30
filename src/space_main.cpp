// =============================================================================
// Big O Demo: Sorting Algorithms -- Space Complexity
// =============================================================================
// Measures the auxiliary (extra) HEAP memory each sorting algorithm allocates
// beyond the input array.  Tracks peak memory using global new/delete
// overrides so you can SEE the space complexity differences:
//
//   O(1) auxiliary:    bubble, insertion, selection -- zero heap allocations
//   O(n) auxiliary:    merge sort (~n ints), radix sort (~n ints)
//   O(k) auxiliary:    counting sort (k = value range, can be >> n)
//   O(n+k) auxiliary:  bucket sort (n ints spread across bucket vectors)
//
// Note: quick sort uses O(log n) STACK space for recursion, which this
// heap tracker does not measure.  Its heap usage is effectively zero.
//
// Not graded -- run it, read the output, and observe the patterns.
// =============================================================================

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// Heap allocation tracking
// -----------------------------------------------------------------------------
// Overrides global operator new/delete to measure peak auxiliary memory.
// Each allocation stores its size in a prefix so delete can subtract it.

static size_t g_heap_current = 0;
static size_t g_heap_peak    = 0;

void reset_peak() { g_heap_peak = g_heap_current; }

void* operator new(size_t size) {
    void* raw = std::malloc(size + sizeof(size_t));
    if (!raw) throw std::bad_alloc();
    std::memcpy(raw, &size, sizeof(size_t));
    g_heap_current += size;
    if (g_heap_current > g_heap_peak) g_heap_peak = g_heap_current;
    return static_cast<char*>(raw) + sizeof(size_t);
}

void operator delete(void* ptr) noexcept {
    if (!ptr) return;
    void* raw = static_cast<char*>(ptr) - sizeof(size_t);
    size_t size;
    std::memcpy(&size, raw, sizeof(size_t));
    g_heap_current -= size;
    std::free(raw);
}

void operator delete(void* ptr, size_t) noexcept { ::operator delete(ptr); }
void* operator new[](size_t size)                 { return ::operator new(size); }
void operator delete[](void* ptr) noexcept        { ::operator delete(ptr); }
void operator delete[](void* ptr, size_t) noexcept{ ::operator delete(ptr); }

// -----------------------------------------------------------------------------
// O(n^2) Sorting Algorithms -- O(1) auxiliary space
// -----------------------------------------------------------------------------

void bubble_sort(std::vector<int>& data) {
    int n = static_cast<int>(data.size());
    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - i; ++j) {
            if (data[j] > data[j + 1]) {
                std::swap(data[j], data[j + 1]);
                swapped = true;
            }
        }
        if (!swapped) break;
    }
}

void insertion_sort(std::vector<int>& data) {
    int n = static_cast<int>(data.size());
    for (int i = 1; i < n; ++i) {
        int key = data[i];
        int j = i - 1;
        while (j >= 0 && data[j] > key) {
            data[j + 1] = data[j];
            --j;
        }
        data[j + 1] = key;
    }
}

void selection_sort(std::vector<int>& data) {
    int n = static_cast<int>(data.size());
    for (int i = 0; i < n - 1; ++i) {
        int min_idx = i;
        for (int j = i + 1; j < n; ++j) {
            if (data[j] < data[min_idx]) min_idx = j;
        }
        if (min_idx != i) std::swap(data[i], data[min_idx]);
    }
}

// -----------------------------------------------------------------------------
// O(n log n) Sorting Algorithms -- O(n) and O(log n) auxiliary
// -----------------------------------------------------------------------------

void merge(std::vector<int>& data, int left, int mid, int right) {
    std::vector<int> left_half(data.begin() + left, data.begin() + mid + 1);
    std::vector<int> right_half(data.begin() + mid + 1, data.begin() + right + 1);
    int i = 0, j = 0, k = left;
    int ls = static_cast<int>(left_half.size());
    int rs = static_cast<int>(right_half.size());
    while (i < ls && j < rs) {
        if (left_half[i] <= right_half[j]) data[k++] = left_half[i++];
        else                                data[k++] = right_half[j++];
    }
    while (i < ls) data[k++] = left_half[i++];
    while (j < rs) data[k++] = right_half[j++];
}

void merge_sort_recursive(std::vector<int>& data, int left, int right) {
    if (left >= right) return;
    int mid = left + (right - left) / 2;
    merge_sort_recursive(data, left, mid);
    merge_sort_recursive(data, mid + 1, right);
    merge(data, left, mid, right);
}

void merge_sort(std::vector<int>& data) {
    if (data.size() <= 1) return;
    merge_sort_recursive(data, 0, static_cast<int>(data.size()) - 1);
}

int partition(std::vector<int>& data, int low, int high) {
    int mid = low + (high - low) / 2;
    if (data[mid] < data[low]) std::swap(data[low], data[mid]);
    if (data[high] < data[low]) std::swap(data[low], data[high]);
    if (data[mid] < data[high]) std::swap(data[mid], data[high]);
    int pivot = data[high];
    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (data[j] <= pivot) { ++i; std::swap(data[i], data[j]); }
    }
    std::swap(data[i + 1], data[high]);
    return i + 1;
}

void quick_sort_recursive(std::vector<int>& data, int low, int high) {
    if (low >= high) return;
    int p = partition(data, low, high);
    quick_sort_recursive(data, low, p - 1);
    quick_sort_recursive(data, p + 1, high);
}

void quick_sort(std::vector<int>& data) {
    if (data.size() <= 1) return;
    quick_sort_recursive(data, 0, static_cast<int>(data.size()) - 1);
}

void heapify_down(std::vector<int>& data, int heap_size, int index) {
    int largest = index;
    int left = 2 * index + 1;
    int right = 2 * index + 2;
    if (left < heap_size && data[left] > data[largest]) largest = left;
    if (right < heap_size && data[right] > data[largest]) largest = right;
    if (largest != index) {
        std::swap(data[index], data[largest]);
        heapify_down(data, heap_size, largest);
    }
}

void heap_sort(std::vector<int>& data) {
    int n = static_cast<int>(data.size());
    for (int i = n / 2 - 1; i >= 0; --i)
        heapify_down(data, n, i);
    for (int i = n - 1; i > 0; --i) {
        std::swap(data[0], data[i]);
        heapify_down(data, i, 0);
    }
}

// -----------------------------------------------------------------------------
// O(n+k) Sorting Algorithms -- O(k) to O(n+k) auxiliary
// -----------------------------------------------------------------------------

void counting_sort(std::vector<int>& data) {
    if (data.size() <= 1) return;
    int min_val = *std::min_element(data.begin(), data.end());
    int max_val = *std::max_element(data.begin(), data.end());
    int range = max_val - min_val + 1;
    std::vector<int> count(range, 0);
    for (int val : data) count[val - min_val]++;
    int idx = 0;
    for (int i = 0; i < range; ++i)
        while (count[i]-- > 0) data[idx++] = i + min_val;
}

void bucket_sort(std::vector<int>& data, int num_buckets = 10) {
    if (data.size() <= 1) return;
    int min_val = *std::min_element(data.begin(), data.end());
    int max_val = *std::max_element(data.begin(), data.end());
    double range = static_cast<double>(max_val - min_val + 1);
    std::vector<std::vector<int>> buckets(num_buckets);
    for (int val : data) {
        int b = static_cast<int>((val - min_val) / range * num_buckets);
        if (b == num_buckets) --b;
        buckets[b].push_back(val);
    }
    int idx = 0;
    for (auto& bucket : buckets) {
        std::sort(bucket.begin(), bucket.end());
        for (int val : bucket) data[idx++] = val;
    }
}

void radix_sort(std::vector<int>& data) {
    if (data.size() <= 1) return;
    int max_val = *std::max_element(data.begin(), data.end());
    int n = static_cast<int>(data.size());
    std::vector<int> output(n);
    for (int exp = 1; max_val / exp > 0; exp *= 10) {
        std::vector<int> count(10, 0);
        for (int val : data) count[(val / exp) % 10]++;
        for (int i = 1; i < 10; ++i) count[i] += count[i - 1];
        for (int i = n - 1; i >= 0; --i) {
            int digit = (data[i] / exp) % 10;
            output[count[digit] - 1] = data[i];
            count[digit]--;
        }
        data = output;
    }
}

// -----------------------------------------------------------------------------
// Test data
// -----------------------------------------------------------------------------

std::vector<int> generate_random_data(int size, unsigned int seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(1, 1'000'000);
    std::vector<int> data(size);
    for (auto& val : data) val = dist(rng);
    return data;
}

// -----------------------------------------------------------------------------
// Space benchmark
// -----------------------------------------------------------------------------

size_t measure_auxiliary(std::function<void(std::vector<int>&)> sort_fn,
                         const std::vector<int>& original) {
    std::vector<int> data = original;   // copy (tracked, but part of baseline)
    size_t before = g_heap_current;     // baseline includes data + original
    reset_peak();                       // peak = current
    sort_fn(data);                      // sort allocates temps -> peak rises
    return g_heap_peak - before;        // peak above baseline = auxiliary
}

// -----------------------------------------------------------------------------
// Output formatting
// -----------------------------------------------------------------------------

void print_header() {
    std::cout << std::setw(20) << "Algorithm"
              << std::setw(14) << "Space"
              << std::setw(10) << "n"
              << std::setw(14) << "Aux Bytes"
              << std::setw(14) << "Aux/n" << "\n";
    std::cout << std::string(72, '-') << "\n";
}

void print_row(const std::string& name, const std::string& complexity,
               int n, size_t bytes) {
    std::cout << std::setw(20) << name
              << std::setw(14) << complexity
              << std::setw(10) << n
              << std::setw(14) << bytes
              << std::setw(14) << std::fixed << std::setprecision(1)
              << static_cast<double>(bytes) / n << "\n";
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main() {
    const std::vector<int> sizes = {1000, 2000, 5000, 10000};

    struct SortAlgorithm {
        std::string name;
        std::string space_complexity;
        std::function<void(std::vector<int>&)> sort_fn;
    };

    std::vector<SortAlgorithm> algorithms = {
        {"Bubble Sort",    "O(1)",   bubble_sort},
        {"Insertion Sort", "O(1)",   insertion_sort},
        {"Selection Sort", "O(1)",   selection_sort},
        {"Merge Sort",     "O(n)",   merge_sort},
        {"Quick Sort",     "O(log n)", quick_sort},
        {"Heap Sort",      "O(1)",   heap_sort},
        {"Counting Sort",  "O(k)",   counting_sort},
        {"Bucket Sort",    "O(n+k)", [](std::vector<int>& d) { bucket_sort(d); }},
        {"Radix Sort",     "O(n)",   radix_sort},
    };

    // Store results: results[algo_index][size_index]
    std::vector<std::vector<size_t>> results(algorithms.size(),
                                              std::vector<size_t>(sizes.size(), 0));

    std::cout << "=============================================================\n";
    std::cout << " Big O Demo: Sorting Algorithms -- Space Complexity\n";
    std::cout << " Measuring auxiliary (extra) heap memory per algorithm\n";
    std::cout << "=============================================================\n";

    // Pre-generate test data
    std::vector<std::vector<int>> test_data;
    for (int size : sizes) {
        test_data.push_back(generate_random_data(size));
    }

    print_header();

    for (size_t a = 0; a < algorithms.size(); ++a) {
        for (size_t s = 0; s < sizes.size(); ++s) {
            size_t aux = measure_auxiliary(algorithms[a].sort_fn, test_data[s]);
            results[a][s] = aux;
            print_row(algorithms[a].name, algorithms[a].space_complexity,
                      sizes[s], aux);
        }
        std::cout << "\n";
    }

    // -------------------------------------------------------------------------
    // Summary table
    // -------------------------------------------------------------------------
    std::cout << "=============================================================\n";
    std::cout << " Summary: Auxiliary Bytes at Each Size\n";
    std::cout << "=============================================================\n";

    std::cout << std::setw(20) << "Algorithm";
    for (int size : sizes) {
        std::cout << std::setw(12) << ("n=" + std::to_string(size));
    }
    std::cout << "\n" << std::string(20 + 12 * sizes.size(), '-') << "\n";

    for (size_t a = 0; a < algorithms.size(); ++a) {
        std::cout << std::setw(20) << algorithms[a].name;
        for (size_t s = 0; s < sizes.size(); ++s) {
            std::cout << std::setw(12) << results[a][s];
        }
        std::cout << "\n";
    }

    // Key lesson
    std::cout << "\n-------------------------------------------------------------\n";
    std::cout << " Key Lesson:\n";
    std::cout << "   O(1):    auxiliary bytes stay constant (zero heap allocs)\n";
    std::cout << "   O(n):    auxiliary bytes grow proportionally to input size\n";
    std::cout << "   O(k):    auxiliary bytes depend on VALUE RANGE, not n\n";
    std::cout << "   Quick sort uses O(log n) STACK space (not tracked here)\n";
    std::cout << "-------------------------------------------------------------\n";

    // -------------------------------------------------------------------------
    // Write CSV
    // -------------------------------------------------------------------------
    std::string repo_dir = REPO_DIR;
    std::string csv_path = repo_dir + "/results_space.csv";
    std::ofstream csv(csv_path);
    csv << "operation,structure,complexity,n,bytes\n";
    for (size_t a = 0; a < algorithms.size(); ++a) {
        for (size_t s = 0; s < sizes.size(); ++s) {
            csv << "sort_space," << algorithms[a].name << ","
                << algorithms[a].space_complexity << "," << sizes[s]
                << "," << results[a][s] << "\n";
        }
    }
    csv.close();

    // -------------------------------------------------------------------------
    // Growth benchmark: larger sizes for O(n) sorts only
    // -------------------------------------------------------------------------
    std::cout << "\n=============================================================\n";
    std::cout << " Growth Benchmark: O(n) sorts at larger sizes\n";
    std::cout << "=============================================================\n";

    const std::vector<int> big_sizes = {10000, 50000, 100000, 250000, 500000};

    struct GrowthAlgorithm {
        std::string name;
        std::string space_complexity;
        std::function<void(std::vector<int>&)> sort_fn;
    };

    std::vector<GrowthAlgorithm> growth_algos = {
        {"Heap Sort",     "O(1)",   heap_sort},
        {"Merge Sort",    "O(n)",   merge_sort},
        {"Bucket Sort",   "O(n+k)", [](std::vector<int>& d) { bucket_sort(d); }},
        {"Counting Sort", "O(k)",   counting_sort},
    };

    // Pre-generate big test data
    std::vector<std::vector<int>> big_data;
    for (int size : big_sizes) {
        big_data.push_back(generate_random_data(size));
    }

    print_header();
    std::string growth_csv_path = repo_dir + "/results_space_growth.csv";
    std::ofstream growth_csv(growth_csv_path);
    growth_csv << "structure,complexity,n,bytes\n";

    for (size_t a = 0; a < growth_algos.size(); ++a) {
        for (size_t s = 0; s < big_sizes.size(); ++s) {
            size_t aux = measure_auxiliary(growth_algos[a].sort_fn, big_data[s]);
            print_row(growth_algos[a].name, growth_algos[a].space_complexity,
                      big_sizes[s], aux);
            growth_csv << growth_algos[a].name << ","
                       << growth_algos[a].space_complexity << ","
                       << big_sizes[s] << "," << aux << "\n";
        }
        std::cout << "\n";
    }
    growth_csv.close();

    std::cout << "  Results written to CSV -- generating charts...\n";

    std::string cmd = "py -3 \"" + repo_dir + "/graph_space.py\" --graph-only";
    std::system(cmd.c_str());

    return 0;
}
