// =============================================================================
// Big O Demo: Sorting Algorithms
// =============================================================================
// Compares O(n^2) vs O(n log n) vs O(n+k) sorting algorithms to show the
// dramatic performance difference as input size grows.
//
// Key takeaway:
// - O(n^2) algorithms: when n doubles, runtime grows ~4x
// - O(n log n) algorithms: when n doubles, runtime grows ~2x
// - O(n+k) algorithms: when n doubles, runtime grows ~2x (linear in n)
// =============================================================================

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <string>
#include <vector>

// -----------------------------------------------------------------------------
// Output formatting
// -----------------------------------------------------------------------------

void print_header(const std::string& title) {
    std::cout << "\n--- " << title << " ---\n";
    std::cout << std::setw(12) << "n"
              << std::setw(15) << "time (us)"
              << std::setw(15) << "growth" << "\n";
    std::cout << std::string(42, '-') << "\n";
}

void print_row(int n, double time_us, double prev_time_us) {
    std::cout << std::setw(12) << n
              << std::setw(15) << std::fixed << std::setprecision(1) << time_us;
    if (prev_time_us > 0) {
        std::cout << std::setw(12) << std::setprecision(1) << (time_us / prev_time_us) << "x";
    }
    std::cout << "\n";
}

// -----------------------------------------------------------------------------
// O(n^2) Sorting Algorithms
// -----------------------------------------------------------------------------

// Bubble Sort
// - Time: O(n^2) average and worst case
// - Space: O(1) — in-place, only swaps adjacent elements
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
        if (!swapped) break;  // Early exit if already sorted
    }
}

// Insertion Sort
// - Time: O(n^2) average and worst case
// - Space: O(1) — in-place, shifts elements one at a time
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

// Selection Sort
// - Time: O(n^2) average and worst case
// - Space: O(1) — in-place, finds minimum and swaps
void selection_sort(std::vector<int>& data) {
    int n = static_cast<int>(data.size());
    for (int i = 0; i < n - 1; ++i) {
        int min_idx = i;
        for (int j = i + 1; j < n; ++j) {
            if (data[j] < data[min_idx]) {
                min_idx = j;
            }
        }
        if (min_idx != i) {
            std::swap(data[i], data[min_idx]);
        }
    }
}

// -----------------------------------------------------------------------------
// O(n log n) Sorting Algorithms
// -----------------------------------------------------------------------------

// Merge Sort (helper: merge two sorted halves)
// - Time: O(n log n) in all cases
// - Space: O(n) — requires temporary array for merging
void merge(std::vector<int>& data, int left, int mid, int right) {
    // - Create temporary vectors for left and right halves
    // - Merge them back in sorted order
    std::vector<int> left_half(data.begin() + left, data.begin() + mid + 1);
    std::vector<int> right_half(data.begin() + mid + 1, data.begin() + right + 1);

    int i = 0, j = 0, k = left;
    int left_size = static_cast<int>(left_half.size());
    int right_size = static_cast<int>(right_half.size());

    while (i < left_size && j < right_size) {
        if (left_half[i] <= right_half[j]) {
            data[k++] = left_half[i++];
        } else {
            data[k++] = right_half[j++];
        }
    }
    while (i < left_size) {
        data[k++] = left_half[i++];
    }
    while (j < right_size) {
        data[k++] = right_half[j++];
    }
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

// Quick Sort
// - Time: O(n log n) average, O(n^2) worst case (rare with good pivot)
// - Space: O(log n) — recursive call stack depth
int partition(std::vector<int>& data, int low, int high) {
    // - Use median-of-three pivot selection to avoid worst-case on sorted data
    int mid = low + (high - low) / 2;
    if (data[mid] < data[low]) std::swap(data[low], data[mid]);
    if (data[high] < data[low]) std::swap(data[low], data[high]);
    if (data[mid] < data[high]) std::swap(data[mid], data[high]);
    int pivot = data[high];

    int i = low - 1;
    for (int j = low; j < high; ++j) {
        if (data[j] <= pivot) {
            ++i;
            std::swap(data[i], data[j]);
        }
    }
    std::swap(data[i + 1], data[high]);
    return i + 1;
}

void quick_sort_recursive(std::vector<int>& data, int low, int high) {
    if (low >= high) return;
    int pivot_idx = partition(data, low, high);
    quick_sort_recursive(data, low, pivot_idx - 1);
    quick_sort_recursive(data, pivot_idx + 1, high);
}

void quick_sort(std::vector<int>& data) {
    if (data.size() <= 1) return;
    quick_sort_recursive(data, 0, static_cast<int>(data.size()) - 1);
}

// -----------------------------------------------------------------------------
// O(n+k) Sorting Algorithms (non-comparison / bucket sorts)
// -----------------------------------------------------------------------------

// Counting Sort
// - Time: O(n + k) where k is the range of input values
// - Space: O(k) for the count array
void counting_sort(std::vector<int>& data) {
    if (data.size() <= 1) return;
    int min_val = *std::min_element(data.begin(), data.end());
    int max_val = *std::max_element(data.begin(), data.end());
    int range = max_val - min_val + 1;

    std::vector<int> count(range, 0);
    for (int val : data) {
        count[val - min_val]++;
    }

    int idx = 0;
    for (int i = 0; i < range; ++i) {
        while (count[i] > 0) {
            data[idx++] = i + min_val;
            --count[i];
        }
    }
}

// Bucket Sort
// - Time: O(n + k) average when data is uniformly distributed
// - Space: O(n + k) for the buckets
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
        for (int val : bucket) {
            data[idx++] = val;
        }
    }
}

// Radix Sort (LSD)
// - Time: O(d * (n + 10)) where d = number of digits
// - Space: O(n + 10)
void radix_sort(std::vector<int>& data) {
    if (data.size() <= 1) return;
    int max_val = *std::max_element(data.begin(), data.end());
    int n = static_cast<int>(data.size());
    std::vector<int> output(n);

    for (int exp = 1; max_val / exp > 0; exp *= 10) {
        std::vector<int> count(10, 0);
        for (int val : data) {
            count[(val / exp) % 10]++;
        }
        for (int i = 1; i < 10; ++i) {
            count[i] += count[i - 1];
        }
        for (int i = n - 1; i >= 0; --i) {
            int digit = (data[i] / exp) % 10;
            output[count[digit] - 1] = data[i];
            count[digit]--;
        }
        data = output;
    }
}

// -----------------------------------------------------------------------------
// Benchmarking utilities
// -----------------------------------------------------------------------------

// Generate a vector of random integers using a fixed seed for reproducibility
std::vector<int> generate_random_data(int size, unsigned int seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> dist(1, 1'000'000);
    std::vector<int> data(size);
    for (auto& val : data) {
        val = dist(rng);
    }
    return data;
}

// Time a sorting function on a copy of the data (microseconds)
double benchmark(std::function<void(std::vector<int>&)> sort_fn,
                 const std::vector<int>& original) {
    // - Work on a copy so the original stays unsorted for the next algorithm
    std::vector<int> data = original;
    auto start = std::chrono::high_resolution_clock::now();
    sort_fn(data);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::micro>(end - start).count();
}

// -----------------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------------

int main() {
    const std::vector<int> sizes = {1000, 2000, 5000, 10000};

    // Define sorting algorithms with names and complexity labels
    struct SortAlgorithm {
        std::string name;
        std::string complexity;
        std::function<void(std::vector<int>&)> sort_fn;
    };

    std::vector<SortAlgorithm> algorithms = {
        {"Bubble Sort",    "O(n^2)",     bubble_sort},
        {"Insertion Sort", "O(n^2)",     insertion_sort},
        {"Selection Sort", "O(n^2)",     selection_sort},
        {"Merge Sort",     "O(n log n)", merge_sort},
        {"Quick Sort",     "O(n log n)", quick_sort},
        {"Counting Sort",  "O(n+k)",     counting_sort},
        {"Bucket Sort",    "O(n+k)",     [](std::vector<int>& d) { bucket_sort(d); }},
        {"Radix Sort",     "O(d(n+k))",  radix_sort},
    };

    // Store results for summary table: results[algo_index][size_index]
    std::vector<std::vector<double>> results(algorithms.size(),
                                              std::vector<double>(sizes.size(), 0.0));

    std::cout << "=============================================================\n";
    std::cout << " Big O Demo: Sorting Algorithms\n";
    std::cout << " Comparing O(n^2) vs O(n log n) vs O(n+k)\n";
    std::cout << "=============================================================\n";

    // Pre-generate test data for each size (same data for all algorithms)
    std::vector<std::vector<int>> test_data;
    for (int size : sizes) {
        test_data.push_back(generate_random_data(size));
    }

    // Benchmark each algorithm
    for (size_t a = 0; a < algorithms.size(); ++a) {
        print_header(algorithms[a].name + " [" + algorithms[a].complexity + "]");
        double prev_time = 0.0;

        for (size_t s = 0; s < sizes.size(); ++s) {
            double time_us = benchmark(algorithms[a].sort_fn, test_data[s]);
            results[a][s] = time_us;
            print_row(sizes[s], time_us, prev_time);
            prev_time = time_us;
        }
    }

    // -------------------------------------------------------------------------
    // Summary comparison table
    // -------------------------------------------------------------------------
    std::cout << "\n=============================================================\n";
    std::cout << " Summary: All Algorithms (times in microseconds)\n";
    std::cout << "=============================================================\n";

    // Header row
    std::cout << std::setw(20) << "Algorithm";
    for (int size : sizes) {
        std::cout << std::setw(12) << ("n=" + std::to_string(size));
    }
    std::cout << "\n" << std::string(20 + 12 * sizes.size(), '-') << "\n";

    // Data rows
    for (size_t a = 0; a < algorithms.size(); ++a) {
        std::cout << std::setw(20) << algorithms[a].name;
        for (size_t s = 0; s < sizes.size(); ++s) {
            std::cout << std::setw(12) << std::fixed << std::setprecision(0) << results[a][s];
        }
        std::cout << "\n";
    }

    // Key lesson
    std::cout << "\n-------------------------------------------------------------\n";
    std::cout << " Key Lesson:\n";
    std::cout << "   O(n^2):      when n doubles, time grows ~4x\n";
    std::cout << "   O(n log n):  when n doubles, time grows ~2x\n";
    std::cout << "   O(n+k):      when n doubles, time grows ~2x (linear)\n";
    std::cout << "-------------------------------------------------------------\n";

    // Write CSV and generate charts
    std::string repo_dir = REPO_DIR;
    std::string csv_path = repo_dir + "/results.csv";
    std::ofstream csv(csv_path);
    csv << "operation,structure,complexity,n,time_us\n";
    for (size_t a = 0; a < algorithms.size(); ++a) {
        for (size_t s = 0; s < sizes.size(); ++s) {
            csv << "sort," << algorithms[a].name << "," << algorithms[a].complexity
                << "," << sizes[s] << "," << std::fixed << std::setprecision(4)
                << results[a][s] << "\n";
        }
    }
    csv.close();
    std::cout << "\n  Results written to CSV -- generating charts...\n";

    std::string cmd = "py -3 \"" + repo_dir + "/graph.py\" --graph-only";
    std::system(cmd.c_str());

    return 0;
}
