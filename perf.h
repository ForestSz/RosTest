#pragma once

#include <chrono>
#include <string>
#include <thread>

#define CALIB_TIME

#if defined(_MSC_VER)
#include <process.h>
#include <stdio.h>
namespace perf {
struct MeasureTime {
  std::string name;
  unsigned long long thread_id;
  unsigned long long process_id;
  inline void log_time(const char *phase) {
    static auto output_file = []() -> std::shared_ptr<FILE> {
      auto profile_path = std::getenv("PROFILE_PATH");
      if (profile_path == nullptr) { // disable output
        return {};
      } else if (std::string(profile_path) == "") { // output to stdout
        return std::shared_ptr<FILE>(stdout, [](FILE *) {});
      } else {
        auto p_file = fopen(profile_path, "w");
        if(p_file == nullptr)
          return nullptr;
        fprintf(p_file, "[");
        return std::shared_ptr<FILE>(
                         p_file, [](FILE *p_file) { fclose(p_file); });
      }
    }();

    if (output_file) {
      _lock_file(output_file.get());
      fprintf(output_file.get(),
              "{\"ts\": %lu, \"pid\":%llu, \"tid\":%llu, \"ph\":\"%s\", "
              "\"name\":\"MeasureTime:%s\"},\n",
              static_cast<unsigned long>(
                  std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count()),
              process_id, thread_id, phase, name.c_str());
      _unlock_file(output_file.get());
    }
  }

  MeasureTime(std::string name)
      : name(name),
        thread_id(std::hash<std::thread::id>{}(std::this_thread::get_id())),
        process_id(_getpid()) {
    log_time("B");
  }
  ~MeasureTime() { log_time("E"); }
};
} // namespace perf
#else
#include <unistd.h>
namespace perf {
struct MeasureTime {
  std::string name;
  unsigned long long thread_id;
  unsigned long long process_id;
  inline void log_time(const char *phase) {
    static auto output_file = []() -> std::shared_ptr<FILE> {
      //auto profile_path = std::getenv("PROFILE_PATH");
      auto profile_path  = "./perf.json";
      if (profile_path == nullptr) { // disable output
        return {};
      } else if (std::string(profile_path) == "") { // output to stdout
        return std::shared_ptr<FILE>(stdout, [](FILE *) {});
      } else {
        auto p_file = fopen(profile_path, "w");
        if(p_file == nullptr)
          return nullptr;
        fprintf(p_file, "[");
        return std::shared_ptr<FILE>(
                         p_file, [](FILE *p_file) { fclose(p_file); });
      }
    }();

    if (output_file) {
      flockfile(output_file.get());
      fprintf(output_file.get(),
              "{\"ts\": %lu, \"pid\":%llu, \"tid\":%llu, \"ph\":\"%s\", "
              "\"name\":\"MeasureTime:%s\"},\n",
              static_cast<unsigned long>(
                  std::chrono::duration_cast<std::chrono::microseconds>(
                      std::chrono::steady_clock::now().time_since_epoch())
                      .count()),
              process_id, thread_id, phase, name.c_str());
      funlockfile(output_file.get());
    }
  }

  MeasureTime(std::string name)
      : name(name),
        thread_id(std::hash<std::thread::id>{}(std::this_thread::get_id())),
        process_id(getpid()) {
    log_time("B");
  }
  ~MeasureTime() { log_time("E"); }
};
} // namespace perf
#endif

namespace perf {
#define MERGE_SUB(X, Y) X##Y
#define MERGE(X, Y) MERGE_SUB(X, Y)

#ifdef CALIB_TIME
#define MEASURE_TIME(name)                                                     \
  perf::MeasureTime MERGE(MEASURE_TIME_, __LINE__)(name);
#else
#define MEASURE_TIME(name)
#endif

/*
class ChronoScope {
 public:
  ChronoScope() {reset();}
  void reset() {start_ = std::chrono::system_clock::now();}
  uint64_t peek_us() const {return duration_cast<std::chrono::microseconds>(
    std::chrono::system_clock::now() - start_).count();
}
  uint64_t peek_ms() const {return duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now() - start_).count();
}
 private:
  std::chrono::system_clock::time_point start_{};
};

*/
} // namespace perf
