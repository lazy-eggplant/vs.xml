/**
 * @file encode-archive.cpp
 * @author random llm model for now I guess
 * @brief 
 * @date 2025-06-21
 * 
 * @copyright Copyright (c) 2025
 * 
 */

//TODO: Adapt it to work with archives

#include <functional>
#include <print>
#include <atomic>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <format>
#include <iostream>
#include <mutex>
#include <optional>
#include <queue>
#include <string_view>
#include <thread>
#include <vector>
#include <stop_token>



struct ProcessResult {
  
    // 1) The actual result codes
    enum Value : int {
      SUCCESS       = 0,
      ERROR_READ    = 1,
      ERROR_PARSE   = 2,
      ERROR_TIMEOUT = 3,
      UNKNOWN       = 4,
      _COUNT        = 5   // sentinel: always last
    };
  
    // 2) How many distinct codes we have
    static constexpr size_t COUNT = static_cast<size_t>(_COUNT);
  
    // 3) Convert a code to its printable name
    static std::string_view to_string(Value v) noexcept {
      switch (v) {
        case SUCCESS:       return "SUCCESS";
        case ERROR_READ:    return "ERROR_READ";
        case ERROR_PARSE:   return "ERROR_PARSE";
        case ERROR_TIMEOUT: return "ERROR_TIMEOUT";
        case UNKNOWN:       return "UNKNOWN";
        default:            return "<?>"; 
      }
    }
  
    // 4) ANSI colors for on-screen display
    static constexpr std::string_view  COLOR_RESET   = "\x1b[0m";
    static constexpr std::string_view  COLOR_GREEN   = "\x1b[32m";
    static constexpr std::string_view  COLOR_YELLOW  = "\x1b[33m";
    static constexpr std::string_view  COLOR_MAGENTA = "\x1b[35m";
    static constexpr std::string_view  COLOR_RED     = "\x1b[31m";
    static constexpr std::string_view  COLOR_CYAN    = "\x1b[36m";
  
    static constexpr std::string_view  color_for(Value v) noexcept {
      switch (v) {
        case SUCCESS:       return COLOR_GREEN;
        case ERROR_READ:    return COLOR_RED;
        case ERROR_PARSE:   return COLOR_RED;
        case ERROR_TIMEOUT: return COLOR_RED;
        case UNKNOWN:       return COLOR_CYAN;
        default:            return COLOR_RESET;
      }
    }
  
    // 5) A handy alias for your atomic counters array
    using CounterArray = std::array<std::atomic<size_t>, COUNT>;
  };


ProcessResult::Value process_file(std::filesystem::path const& p) {
    // stub: hash path to pick a pseudo‐random result
    auto h = std::hash<std::string>{}(p.string());
    return static_cast<ProcessResult::Value>(h % static_cast<int>(ProcessResult::_COUNT));
}

//------------------------------------------------------------------------------
// Minimal thread‐pool using std::jthread + stop_token
//------------------------------------------------------------------------------
class ThreadPool {
public:
  explicit ThreadPool(size_t n) {
    for (size_t i = 0; i < n; ++i) {
      workers_.emplace_back([this](std::stop_token st) {
        while (!st.stop_requested()) {
          std::function<void()> job;
          {
            std::unique_lock lock{mtx_};
            cv_.wait(lock, [&]{ 
              // wake up either on new task or on our own stop request
              return st.stop_requested() || !tasks_.empty();
            });
            if (st.stop_requested() && tasks_.empty())
              return;   // clean exit
            job = std::move(tasks_.front());
            tasks_.pop();
          }
          job();
        }
      });
    }
  }

  ~ThreadPool() {
    // signal each jthread to stop
    for (auto &jt : workers_) {
      jt.request_stop();
    }
    cv_.notify_all();
    // jthreads auto‐join on destruction
  }

  void submit(std::function<void()> job) {
    {
      std::lock_guard lock{mtx_};
      tasks_.push(std::move(job));
    }
    cv_.notify_one();
  }

private:
  std::vector<std::jthread>             workers_;
  std::queue<std::function<void()>>     tasks_;
  std::mutex                            mtx_;
  std::condition_variable_any           cv_;
};

//------------------------------------------------------------------------------
// Simple CLI parsing
//------------------------------------------------------------------------------
struct Config {
    std::filesystem::path  source;
    std::optional<std::filesystem::path> log_path;
    unsigned               threads     = std::thread::hardware_concurrency();
    bool                   want_report = false;
};

std::optional<Config> parse_args(int argc, char* argv[]) {
    if (argc < 2) return std::nullopt;
    Config cfg;
    cfg.source = argv[1];
    for (int i = 2; i < argc; ++i) {
        std::string_view a = argv[i];
        if (a == "--log" && i+1 < argc) {
            cfg.log_path = argv[++i];
        }
        else if (a == "--threads" && i+1 < argc) {
            unsigned t = std::stoul(argv[++i]);
            cfg.threads = t>0? t:1;
        }
        else if (a == "--report") {
            cfg.want_report = true;
        }
        else {
            std::print(stderr, "Unknown option: '{}'\n", a);
            return std::nullopt;
        }
    }
    return cfg;
}

//------------------------------------------------------------------------------
// main
//------------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    auto cfg_opt = parse_args(argc, argv);
    if (!cfg_opt) {
        std::print("Usage: {} <source_dir> [--log <path>] [--threads N] [--report]\n", argv[0]);
        return 1;
    }
    auto const& cfg = *cfg_opt;

    if (!std::filesystem::exists(cfg.source) ||
        !std::filesystem::is_directory(cfg.source)) {
        std::print(stderr, "Error: '{}' is not a directory\n", cfg.source.string());
        return 2;
    }

    // Optional extended log (failures only)
    std::optional<std::ofstream> log_stream;
    if (cfg.log_path) {
        log_stream.emplace(cfg.log_path->c_str(), std::ios::out);
        if (!*log_stream) {
            std::print(stderr, "Error: cannot open log file '{}'\n", cfg.log_path->string());
            return 3;
        }
    }

    // 1) Gather files
    std::vector<std::filesystem::path> files;
    try {
        for (auto const& e : std::filesystem::recursive_directory_iterator(cfg.source)) {
            if (e.is_regular_file())
                files.push_back(e.path());
        }
    }
    catch (std::exception const& ex) {
        std::print(stderr, "Error scanning '{}': {}\n", cfg.source.string(), ex.what());
        return 4;
    }

    // 2) Counters (fixed at compile time by ProcessResult::_COUNT)
    constexpr size_t RCOUNT = static_cast<size_t>(ProcessResult::_COUNT);
    ProcessResult::CounterArray counters;
    for (auto &c : counters)c.store(0, std::memory_order_relaxed);

    // 3) Thread‐pool
    ThreadPool pool(cfg.threads);

    // 4) Submit tasks
    for (auto const& path : files) {
        pool.submit([&, path]() {
            auto res = process_file(path);
            counters[static_cast<size_t>(res)]
              .fetch_add(1, std::memory_order_relaxed);

            // **Extended log**: only record failures (res != SUCCESS)
            if (log_stream && res != ProcessResult::SUCCESS) {
                std::print(*log_stream, "{} => {}\n",
                           path.string(), ProcessResult::to_string(res));
            }
        });
    }
    // pool destructor waits for all tasks to finish

    // 5) On‐screen short report (counts only), with colors
    if (cfg.want_report) {
        size_t total = 0;
        for (auto& c : counters) total += c.load();

        std::print("=== Processing Report ===\n");
        std::print("Source dir: {}\n", cfg.source.string());
        std::print("Total files processed: {}\n\n", total);
        std::print("Result breakdown:\n");

        for (size_t i = 0; i < RCOUNT; ++i) {
            auto r     = static_cast<ProcessResult::Value>(i);
            //auto cnt   = counters[i].load();
            auto name  = ProcessResult::to_string(r);
            auto color = ProcessResult::color_for(r);
            std::print("{}{:<16}{} : {}\n",
                color, name, ProcessResult::COLOR_RESET,
                counters[static_cast<size_t>(r)].load());
        }
    }

    size_t failures = 0;
    for (size_t i = 0; i < ProcessResult::COUNT; ++i) {
        size_t c = counters[i].load(std::memory_order_relaxed);
        if (static_cast<ProcessResult::Value>(i) != ProcessResult::SUCCESS)
            failures += c;
    }

    return failures==0?0:1;
}