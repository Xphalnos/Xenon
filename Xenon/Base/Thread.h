// Copyright 2025 Xenon Emulator Project. All rights reserved.

#pragma once

#include <chrono>

namespace Base {

enum class ThreadPriority : u32 {
  Low = 0,
  Normal = 1,
  High = 2,
  VeryHigh = 3,
  Critical = 4
};

void SetCurrentThreadRealtime(std::chrono::nanoseconds period_ns);

void SetCurrentThreadPriority(ThreadPriority new_priority);

void SetCurrentThreadName(const std::string_view &name);

void SetThreadName(void *thread, const std::string_view &name);

class AccurateTimer {
  std::chrono::nanoseconds target_interval{};
  std::chrono::nanoseconds total_wait{};

  std::chrono::high_resolution_clock::time_point start_time;

public:
  explicit AccurateTimer(std::chrono::nanoseconds target_interval);

  void Start();

  void End();

  std::chrono::nanoseconds GetTotalWait() const {
    return total_wait;
  }
};

} // namespace Base
