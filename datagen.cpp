// MIT License

// Copyright (c) 2023 Mayur Pise (mayurpise@gmail.com)

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <iostream>
#include <filesystem>
#include <thread>
#include <vector>
#include <functional>
#include <queue>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <cmath>
#include <random>
#include <sstream>
#include <future>
#include <iostream>
#include <dirent.h>
#include <cstring>
#include <string>


namespace fs = std::filesystem;

// ThreadPool implementation
class ThreadPool {
public:
    ThreadPool(size_t threads);
    ~ThreadPool();

    template <class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))>;

    void waitFinished();

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;

    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_ = false;
    std::atomic<int> working_threads_ = 0;

    void workerFunction();
};

ThreadPool::ThreadPool(size_t threads) {
    for (size_t i = 0; i < threads; ++i)
        workers_.emplace_back(&ThreadPool::workerFunction, this);
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    condition_.notify_all();
    for (auto& worker : workers_)
        worker.join();
}

template <class F, class... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    using return_type = decltype(f(args...));

    auto task = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
    );

    std::future<return_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("enqueue on stopped ThreadPool");
        }
        tasks_.emplace([task]() { (*task)(); });
    }
    condition_.notify_one();
    return res;
}

void ThreadPool::waitFinished() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    condition_.wait(lock, [this] { return tasks_.empty() && (working_threads_ == 0); });
}

void ThreadPool::workerFunction() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });

            if (stop_ && tasks_.empty())
                return;

            task = std::move(tasks_.front());
            tasks_.pop();
            ++working_threads_;
        }

        task();

        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            --working_threads_;
            if (tasks_.empty() && working_threads_ == 0) {
                condition_.notify_all();
            }
        }
    }
}

// Function to write random data to a file
void writeToFile(const fs::path& filepath, size_t fileSizeKB) {
    // Create a random generator
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<char> dist(' ', '~'); // printable ASCII characters

    std::stringstream content;
    for (size_t i = 0; i < 1024 * fileSizeKB; i++) {
        content << dist(mt);
    }

    std::ofstream out(filepath);
    out << content.str();
    out.close();
}


std::pair<int, int> createData(const std::string& parentDir, size_t numLevels, size_t numDirsPerLevel,
                               size_t numFilesPerDir, size_t fileSizeKB, size_t currentLevel,
                               size_t targetLevel, size_t numFilesAtTargetLevel, ThreadPool& pool) {
    if (currentLevel > numLevels)
        return {0, 0};

    int localDirsCreated = 0;
    int localFilesCreated = 0;

    // Create numDirsPerLevel directories at the current level
    std::vector<fs::path> directoriesToCreate;
    for (size_t i = 0; i < numDirsPerLevel; i++) {
        std::string dirName = "dir_" + std::to_string(currentLevel) + "_" + std::to_string(i);
        fs::path newDir = fs::path(parentDir).append(dirName);
        directoriesToCreate.push_back(newDir);
    }

    try {
        for (const auto& newDir : directoriesToCreate) {
            fs::create_directories(newDir);
            localDirsCreated++;
        }

        // Create files in parallel threads
        std::vector<std::future<void>> futures;
        for (size_t i = 0; i < numDirsPerLevel; i++) {
            futures.push_back(pool.enqueue([=, &localFilesCreated]() {
                // Determine the number of files to create based on the current level
                size_t filesToCreate = (currentLevel == targetLevel) ? numFilesAtTargetLevel : numFilesPerDir;

                for (size_t j = 0; j < filesToCreate; j++) {
                    fs::path filePath = directoriesToCreate[i] / ("file_" + std::to_string(j));
                    writeToFile(filePath, fileSizeKB);
                    localFilesCreated++;
                }
            }));
        }

        // Wait for all threads to finish
        for (auto& future : futures) {
            future.get();
        }

        // Recursively create data for subdirectories
        for (size_t i = 0; i < numDirsPerLevel; i++) {
            auto [dirs, files] = createData(directoriesToCreate[i].string(), numLevels, numDirsPerLevel,
                                            numFilesPerDir, fileSizeKB, currentLevel + 1,
                                            targetLevel, numFilesAtTargetLevel, pool);
            localDirsCreated += dirs;
            localFilesCreated += files;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error creating directories or files: " << e.what() << std::endl;
    }

    return {localDirsCreated, localFilesCreated};
}

// The function writeToFile and ThreadPool class remains unchanged.


size_t calculateTotalFiles(size_t numLevels, size_t numDirsPerLevel, size_t numFilesPerDir,
                           size_t currentLevel, size_t targetLevel, size_t numFilesAtTargetLevel,
                           size_t &sumOfAllFiles) {
    size_t totalFiles = 0;
    size_t filesAtLevel = 0;
    if (currentLevel == targetLevel) {
        filesAtLevel = numDirsPerLevel * numFilesAtTargetLevel;
    } else if (currentLevel < numLevels) {
        filesAtLevel = numDirsPerLevel * calculateTotalFiles(numLevels, numDirsPerLevel, numFilesPerDir,
                                                     currentLevel + 1, targetLevel, numFilesAtTargetLevel, sumOfAllFiles);
    } else {
        return 0; // Current level is beyond the target level, no files here.
    }
    totalFiles += filesAtLevel;
    sumOfAllFiles += totalFiles;
    return totalFiles;
}


size_t calculateTotalDirectories(size_t numLevels, size_t numDirsPerLevel) {
    size_t totalDirs = 0;

    for (size_t i = 1; i <= numLevels; ++i) {
        totalDirs += std::pow(numDirsPerLevel, i);
    }

    return totalDirs;
}

struct RoundResult {
    int round;
    double iops;
    long duration;
};


class DirectoryQuerier {
public:
    DirectoryQuerier(const std::string& target_dir, size_t num_threads, bool use_concurrency)
        : target_dir_(target_dir), pool_(num_threads), use_concurrency_(use_concurrency) {}

    void queryDirectoryRecursively(std::atomic<size_t>& file_count, std::atomic<size_t>& dir_count);

private:
    std::string target_dir_;
    ThreadPool pool_;
    std::atomic<int> active_tasks_{ 0 };
    std::condition_variable cv_;
    std::mutex cv_m_;
    bool use_concurrency_; // Flag to toggle between concurrent and sequential

    void traverseDirectory(const fs::path& path, std::atomic<size_t>& file_count, std::atomic<size_t>& dir_count);
    void traverseDirectoryTask(const fs::path& path, std::atomic<size_t>& file_count, std::atomic<size_t>& dir_count);
};

void DirectoryQuerier::queryDirectoryRecursively(std::atomic<size_t>& file_count, std::atomic<size_t>& dir_count) {
    {
        std::lock_guard<std::mutex> lk(cv_m_);
        traverseDirectory(target_dir_, file_count, dir_count);
    }

    if (use_concurrency_) {
        std::unique_lock<std::mutex> lk(cv_m_);
        cv_.wait(lk, [this] { return active_tasks_ == 0; });
    }
}

void DirectoryQuerier::traverseDirectory(const fs::path& path, std::atomic<size_t>& file_count, std::atomic<size_t>& dir_count) {
    if (use_concurrency_) {
        active_tasks_++;
        pool_.enqueue([this, path, &file_count, &dir_count]() {
            this->traverseDirectoryTask(path, file_count, dir_count);
        });
    } else {
        traverseDirectoryTask(path, file_count, dir_count);
    }
}

void DirectoryQuerier::traverseDirectoryTask(const fs::path& path, std::atomic<size_t>& file_count, std::atomic<size_t>& dir_count) {
    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            if (fs::is_directory(entry.status())) {
                dir_count++;
                traverseDirectory(entry.path(), file_count, dir_count);
            } else if (fs::is_regular_file(entry.status())) {
                file_count++;
                // Get the file name and print it (optional)
                // std::cout << "File Name: " << entry.path().filename() << "\n";
                // fs::path file_path = entry.path();
                // Read metadata (optional)
                // auto file_size = fs::file_size(file_path);
            }
            // Handle symlinks and other file types here if necessary
        }
    } catch (const fs::filesystem_error& e) {
        // Handle exceptions (e.g., permission denied, path not found)
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } catch (const std::exception& e) {
        // Handle other exceptions
        std::cerr << "Error: " << e.what() << std::endl;
    }

    if (use_concurrency_) {
        std::lock_guard<std::mutex> lk(cv_m_);
        active_tasks_--;
        cv_.notify_one();
    }
}

// Main function
int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: ./datagen <w|rw> <target_dir> <num_levels> <num_dirs_per_level> "
                     "<num_files_per_dir> <file_size_kb> <target_level> <num_files_at_target_level>" << std::endl;
        std::cerr << "Usage: ./datagen <r> <target_dir> <test_rounds>" << std::endl;
        std::cerr << "Note: You can set and export NUM_THREADS, default is hardware concurrency" << std::endl;
        return 1;
    }

    std::string mode = argv[1];
    int col_width = 25;

    struct DatagenPerformanceStats {
        std::string target_dir;
        long total_dirs = 0;
        long total_files = 0;
        long total_files_dirs = 0;
        long write_duration = 0;
        long write_iops = 0;
        float write_response_time = 0;
        long read_iops = 0;
        float read_response_time = 0;
        long read_duration = 0;
    };

    DatagenPerformanceStats datagen_perf_stats;

    if (mode == "w" || mode == "rw") {
        if (argc != 9) {
            std::cerr << "Usage: ./datagen <w> <target_dir> <num_levels> <num_dirs_per_level> "
                "<num_files_per_dir> <file_size_kb> <target_level> <num_files_at_target_level>"<< std::endl;
                std::cerr << "Note: You can set and export NUM_THREADS, default is hardware concurrency" << std::endl;
            return 1;
        }

        std::string target_dir = argv[2];
        size_t numLevels = std::stoull(argv[3]);
        size_t numDirsPerLevel = std::stoull(argv[4]);
        size_t numFilesPerDir = std::stoull(argv[5]);
        size_t fileSizeKB = std::stoull(argv[6]);
        size_t targetLevel = std::stoull(argv[7]);
        size_t numFilesAtTargetLevel = std::stoull(argv[8]);

        const char* numThreadsEnv = std::getenv("NUM_THREADS");
        int numThreads = numThreadsEnv ? std::atoi(numThreadsEnv) : std::thread::hardware_concurrency();

        ThreadPool pool(numThreads);
        auto start_write = std::chrono::steady_clock::now();

        auto [totalDirsCreated, totalFilesCreated] = createData(target_dir, numLevels, numDirsPerLevel, numFilesPerDir, fileSizeKB,
                    1, targetLevel, numFilesAtTargetLevel, pool);

        auto end_write = std::chrono::steady_clock::now();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_write - start_write);
        long long time_difference_ms = duration.count();
        double time_difference_sec = time_difference_ms / 1000.0;

        long iops = (totalFilesCreated + totalDirsCreated) / time_difference_sec;
        int totalFilesDirs = totalFilesCreated + totalDirsCreated;
        float avgResponseTime = static_cast<float>(time_difference_ms) / totalFilesDirs;

        datagen_perf_stats.target_dir = target_dir;
        datagen_perf_stats.total_files = totalFilesCreated;
        datagen_perf_stats.total_dirs = totalDirsCreated;
        datagen_perf_stats.total_files_dirs = totalFilesDirs;
        datagen_perf_stats.write_duration = time_difference_ms;
        datagen_perf_stats.write_iops = iops;
        datagen_perf_stats.write_response_time = avgResponseTime;
    }

    if (mode == "r" || mode == "rw") {
        // if (argc != 3) {
        //     std::cerr << "Usage: ./datagen <r> <target_dir> <test_rounds>" << std::endl;
        //     std::cerr << "Note: You can set and export NUM_THREADS, default is hardware concurrency" << std::endl;
        //     return 1;
        // }

        std::string target_dir = argv[2];
        int test_rounds = std::stoi(argv[3]);

        unsigned int hardware_concurrency = std::thread::hardware_concurrency();

        std::vector<RoundResult> allRoundResults;
        std::vector<long> durations;
        double total_iops = 0;
        size_t last_round_files = 0;
        size_t last_round_dirs = 0;

        for (int i = 0; i < test_rounds; i++) {
            DirectoryQuerier querier(target_dir, hardware_concurrency, true);

            std::atomic<size_t> round_files(0);
            std::atomic<size_t> round_dirs(0);

            auto start_read = std::chrono::steady_clock::now();
            querier.queryDirectoryRecursively(round_files, round_dirs);
            auto end_read = std::chrono::steady_clock::now();

            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_read - start_read).count();
            double time_difference_sec = duration / 1000.0;

            durations.push_back(duration);

            double iops = (round_files.load() + round_dirs.load()) / time_difference_sec;
            RoundResult currentRoundResult{ i + 1, iops, duration };
            allRoundResults.push_back(currentRoundResult);

            total_iops += iops;

            last_round_files = round_files.load();
            last_round_dirs = round_dirs.load();

            int totalFilesDirs = last_round_files + last_round_dirs;
            float avgResponseTime = static_cast<float>(duration) / totalFilesDirs;

            std::cout << "Round " << i + 1 << " - Files: " << last_round_files
                      << ", Dirs: " << last_round_dirs << ", Duration: " << duration
                      << " ms, Read IOPS: " << iops << ", Response Time: " << avgResponseTime << " ms" << std::endl;
        }

        long total_duration = std::accumulate(durations.begin(), durations.end(), 0L);
        long avgDuration = total_duration / test_rounds;
        long avgIops = total_iops / test_rounds;

        int totalFilesDirs = last_round_files + last_round_dirs;
        float avgResponseTime = static_cast<float>(avgDuration) / totalFilesDirs;

        datagen_perf_stats.target_dir = target_dir;
        datagen_perf_stats.total_files = last_round_files;
        datagen_perf_stats.total_dirs = last_round_dirs;
        datagen_perf_stats.total_files_dirs = totalFilesDirs;
        datagen_perf_stats.read_duration = avgDuration;
        datagen_perf_stats.read_iops = avgIops;
        datagen_perf_stats.read_response_time = avgResponseTime;

        std::cout << "Average - Files: " << last_round_files << ", Dirs: " << last_round_dirs
                  << ", Duration: " << avgDuration << " ms, Read IOPS: " << avgIops
                  << ", Response Time: " << avgResponseTime << " ms" << std::endl;

        std::cout << "----------------------------------------------------------" << std::endl;
    }

    col_width = 30;
    std::cout << std::left << std::setw(col_width) << "Target Directory: " << datagen_perf_stats.target_dir << std::endl;
    std::cout << std::left << std::setw(col_width) << "Total Dirs: " << datagen_perf_stats.total_dirs << std::endl;
    std::cout << std::left << std::setw(col_width) << "Total Files: " << datagen_perf_stats.total_files << std::endl;
    std::cout << std::left << std::setw(col_width) << "Total Files & Dirs: " << datagen_perf_stats.total_files_dirs << std::endl;
    std::cout << std::left << std::setw(col_width) << "Write Duration ms: " << datagen_perf_stats.write_duration << std::endl;
    std::cout << std::left << std::setw(col_width) << "Write IOPS: " << datagen_perf_stats.write_iops << std::endl;
    std::cout << std::left << std::setw(col_width) << "Write Response Time ms: " << datagen_perf_stats.write_response_time << std::endl;
    std::cout << std::left << std::setw(col_width) << "Read Duration ms: " << datagen_perf_stats.read_duration << std::endl;
    std::cout << std::left << std::setw(col_width) << "Read IOPS: " << datagen_perf_stats.read_iops << std::endl;
    std::cout << std::left << std::setw(col_width) << "Read Response Time ms: " << datagen_perf_stats.read_response_time << std::endl;

    return 0;
}