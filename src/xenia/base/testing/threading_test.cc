/**
******************************************************************************
* Xenia : Xbox 360 Emulator Research Project                                 *
******************************************************************************
* Copyright 2018 Ben Vanik. All rights reserved.                             *
* Released under the BSD license - see LICENSE in the root for more details. *
******************************************************************************
*/

#include "xenia/base/threading.h"

#include "third_party/catch/include/catch.hpp"

#include <array>

namespace xe {
namespace base {
namespace test {
using namespace threading;
using namespace std::chrono_literals;

class InvalidHandle : public threading::WaitHandle {
 public:
  explicit InvalidHandle() {}

 protected:
  void* native_handle() const override { return nullptr; }
};

TEST_CASE("Signal and Wait", "Fence") {
  using namespace std::chrono_literals;

  std::unique_ptr<threading::Fence> pFence;
  std::unique_ptr<threading::HighResolutionTimer> pTimer;

  // Signal without wait
  pFence = std::make_unique<threading::Fence>();
  pFence->Signal();

  // Signal once and wait
  pFence = std::make_unique<threading::Fence>();
  pFence->Signal();
  pFence->Wait();

  // Signal twice and wait
  pFence = std::make_unique<threading::Fence>();
  pFence->Signal();
  pFence->Signal();
  pFence->Wait();

  // TODO(bwrsandman): Test fence on thread or timer
}

TEST_CASE("Get number of logical processors") {
  auto count = std::thread::hardware_concurrency();
  REQUIRE(logical_processor_count() == count);
  REQUIRE(logical_processor_count() == count);
  REQUIRE(logical_processor_count() == count);
}

TEST_CASE("Enable process to set thread affinity") {
  EnableAffinityConfiguration();
}

TEST_CASE("Yield Current Thread", "MaybeYield") {
  // Run to see if there are any errors
  MaybeYield();
}

TEST_CASE("Sync with Memory Barrier", "SyncMemory") {
  // Run to see if there are any errors
  SyncMemory();
}

TEST_CASE("Sleep Current Thread", "Sleep") {
  auto wait_time = 50ms;
  auto start = std::chrono::steady_clock::now();
  Sleep(wait_time);
  auto duration = std::chrono::steady_clock::now() - start;
  REQUIRE(duration >= wait_time);
}

TEST_CASE("Sleep Current Thread in Alertable State", "Sleep") {
  // TODO(bwrsandman):
  REQUIRE(true == true);
}

TEST_CASE("TlsHandle") {
  // TODO(bwrsandman):
  REQUIRE(true == true);
}

TEST_CASE("HighResolutionTimer") {
  // The wait time is 500ms with an interval of 50ms
  // Smaller values are not as precise and fail the test
  const auto wait_time = 500ms;

  // Time the actual sleep duration
  {
    const auto interval = 50ms;
    uint64_t counter = 0;
    auto start = std::chrono::steady_clock::now();
    auto cb = [&counter] { ++counter; };
    auto pTimer = HighResolutionTimer::CreateRepeating(interval, cb);
    Sleep(wait_time);
    pTimer.reset();
    auto duration = std::chrono::steady_clock::now() - start;

    // Should have run as many times as wait_time / timer_interval plus or
    // minus 1 due to imprecision of Sleep
    REQUIRE(duration.count() >= wait_time.count());
    auto ratio = static_cast<uint64_t>(duration / interval);
    REQUIRE(counter >= ratio - 1);
    REQUIRE(counter <= ratio + 1);
  }

  // Test concurrent timers
  {
    const auto interval1 = 10ms;
    const auto interval2 = 20ms;
    uint64_t counter1 = 0;
    uint64_t counter2 = 0;
    auto start = std::chrono::steady_clock::now();
    auto cb1 = [&counter1] { ++counter1; };
    auto cb2 = [&counter2] { ++counter2; };
    auto pTimer1 = HighResolutionTimer::CreateRepeating(interval1, cb1);
    auto pTimer2 = HighResolutionTimer::CreateRepeating(interval2, cb2);
    Sleep(wait_time);
    pTimer1.reset();
    pTimer2.reset();
    auto duration = std::chrono::steady_clock::now() - start;

    // Should have run as many times as wait_time / timer_interval plus or
    // minus 1 due to imprecision of Sleep
    REQUIRE(duration.count() >= wait_time.count());
    auto ratio1 = static_cast<uint64_t>(duration / interval1);
    auto ratio2 = static_cast<uint64_t>(duration / interval2);
    REQUIRE(counter1 >= ratio1 - 1);
    REQUIRE(counter1 <= ratio1 + 1);
    REQUIRE(counter2 >= ratio2 - 1);
    REQUIRE(counter2 <= ratio2 + 1);
  }

  // TODO(bwrsandman): Check on which thread callbacks are executed when
  // spawned from differing threads
}

TEST_CASE("Wait on Handle", "Wait") {
  using namespace std::chrono_literals;

  std::unique_ptr<threading::Thread> thread;
  threading::WaitResult result;
  threading::Thread::CreationParameters params = {};
  auto func = [] { threading::Sleep(20ms); };

  // Call wait on simple thread
  thread = threading::Thread::Create(params, func);
  result = threading::Wait(thread.get(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kSuccess);

  // Timeout on simple thread
  thread = threading::Thread::Create(params, func);
  result = threading::Wait(thread.get(), false, 1ms);
  REQUIRE(result == threading::WaitResult::kTimeout);
  result = threading::Wait(thread.get(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kSuccess);

  // Call wait on invalid handle
  // TODO(bwrsandman)
  // auto invalid_handle = std::make_unique<InvalidHandle>();
  // result = threading::Wait(invalid_handle.get(), false, 50ms);
  // REQUIRE(result == threading::WaitResult::kFailed);
}

TEST_CASE("Wait for all of Multiple Handles", "Wait") {
  /* using namespace std::chrono_literals;

  std::unique_ptr<threading::Thread> thread0, thread1, thread2;
  std::vector<threading::WaitHandle*> threads;
  threading::WaitResult result;
  threading::Thread::CreationParameters params = {};
  auto func0 = [] { threading::Sleep(30ms); };
  auto func1 = [] { threading::Sleep(20ms); };
  auto func2 = [] { threading::Sleep(10ms); };

  // Call wait on simple thread
  thread0 = threading::Thread::Create(params, func0);
  thread1 = threading::Thread::Create(params, func1);
  thread2 = threading::Thread::Create(params, func2);
  threads.push_back(thread0.get());
  threads.push_back(thread1.get());
  threads.push_back(thread2.get());
  result = threading::WaitAll(threads, false, 50ms);
  REQUIRE(result == threading::WaitResult::kSuccess);
  threads.clear();

  // Timeout on simple thread
  thread0 = threading::Thread::Create(params, func0);
  thread1 = threading::Thread::Create(params, func1);
  thread2 = threading::Thread::Create(params, func2);
  threads.push_back(thread0.get());
  threads.push_back(thread1.get());
  threads.push_back(thread2.get());
  result = threading::WaitAll(threads, false, 0ms);
  REQUIRE(result == threading::WaitResult::kTimeout);
  result = threading::WaitAll(threads, false, 20ms);
  REQUIRE(result == threading::WaitResult::kTimeout);
  result = threading::WaitAll(threads, false, 50ms);
  REQUIRE(result == threading::WaitResult::kSuccess);
  threads.clear();

  thread0 = threading::Thread::Create(params, func0);
  thread1 = threading::Thread::Create(params, func1);
  thread2 = threading::Thread::Create(params, func2);
  auto invalid_handle = std::make_unique<InvalidHandle>();
  threads.push_back(thread0.get());
  threads.push_back(thread1.get());
  threads.push_back(thread2.get());
  threads.push_back(invalid_handle.get());
  result = threading::WaitAll(threads, false, 50ms);
  REQUIRE(result == threading::WaitResult::kFailed);
  threads.clear();*/
}

TEST_CASE("Wait for any of Multiple Handles", "Wait") {
  /* using namespace std::chrono_literals;

  std::unique_ptr<threading::Thread> thread0, thread1, thread2;
  std::vector<threading::WaitHandle*> threads;
  std::pair<threading::WaitResult, size_t> result;
  threading::Thread::CreationParameters params = {};
  auto func0 = [] { threading::Sleep(30ms); };
  auto func1 = [] { threading::Sleep(20ms); };
  auto func2 = [] { threading::Sleep(10ms); };

  // Call wait on simple thread
  thread0 = threading::Thread::Create(params, func0);
  thread1 = threading::Thread::Create(params, func1);
  thread2 = threading::Thread::Create(params, func2);
  threads.push_back(thread0.get());
  threads.push_back(thread1.get());
  threads.push_back(thread2.get());
  result = threading::WaitAny(threads, false, 50ms);
  REQUIRE(result.first == threading::WaitResult::kSuccess);
  REQUIRE(result.second == 2);  // Shortest sleep of 10ms
  threads.clear();

  // Timeout on simple thread
  thread0 = threading::Thread::Create(params, func0);
  thread1 = threading::Thread::Create(params, func1);
  thread2 = threading::Thread::Create(params, func2);
  threads.push_back(thread0.get());
  threads.push_back(thread1.get());
  threads.push_back(thread2.get());
  result = threading::WaitAny(threads, false, 0ms);
  REQUIRE(result.first == threading::WaitResult::kTimeout);
  REQUIRE(result.second == 0);
  result = threading::WaitAny(threads, false, 20ms);
  REQUIRE(result.first == threading::WaitResult::kSuccess);
  REQUIRE(result.second == 2);  // Shortest sleep of 10ms
  result = threading::WaitAny(threads, false, 50ms);
  REQUIRE(result.first == threading::WaitResult::kSuccess);
  REQUIRE(result.second == 2);  // Shortest sleep of 10ms
  threads.clear();

  // Call wait on invalid handle
  thread0 = threading::Thread::Create(params, func0);
  thread1 = threading::Thread::Create(params, func1);
  thread2 = threading::Thread::Create(params, func2);
  auto invalid_handle = std::make_unique<InvalidHandle>();
  threads.push_back(thread0.get());
  threads.push_back(thread1.get());
  threads.push_back(thread2.get());
  threads.push_back(invalid_handle.get());
  result = threading::WaitAny(threads, false, 50ms);
  REQUIRE(result.first == threading::WaitResult::kFailed);
  REQUIRE(result.second == 0);  // Failures always return 0
  threads.clear();*/
}

TEST_CASE("Wait on Event", "Event") {
  auto evt = Event::CreateAutoResetEvent(false);
  WaitResult result;

  // Call wait on unset Event
  result = Wait(evt.get(), false, 50ms);
  REQUIRE(result == WaitResult::kTimeout);

  // Call wait on set Event
  evt->Set();
  result = Wait(evt.get(), false, 50ms);
  REQUIRE(result == WaitResult::kSuccess);

  // Call wait on now consumed Event
  result = Wait(evt.get(), false, 50ms);
  REQUIRE(result == WaitResult::kTimeout);

  // Call wait on reset Event
  evt->Set();
  evt->Reset();
  result = xe::threading::Wait(evt.get(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kTimeout);

  // Test resetting the unset event
  evt->Reset();
  result = xe::threading::Wait(evt.get(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kTimeout);

  // Test setting the reset event
  evt->Set();
  result = xe::threading::Wait(evt.get(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kSuccess);

  // TODO(bwrsandman): test Pulse()
}

TEST_CASE("Wait on Semaphore", "Semaphore") {
  // TODO(bwrsandman):
  REQUIRE(true == true);
}

TEST_CASE("Wait on Mutant", "Mutant") {
  // TODO(bwrsandman):
  REQUIRE(true == true);
}

TEST_CASE("Create and Trigger Timer", "Timer") {
  // TODO(bwrsandman):
  REQUIRE(true == true);
}

TEST_CASE("Set and Test Current Thread ID", "Thread") {
  // System ID
  auto system_id = current_thread_system_id();
  REQUIRE(system_id > 0);

  // Thread ID
  auto thread_id = current_thread_id();
  REQUIRE(thread_id == system_id);

  // Set a new thread id
  const uint32_t new_thread_id = 0xDEADBEEF;
  set_current_thread_id(new_thread_id);
  REQUIRE(current_thread_id() == new_thread_id);

  // Set back original thread id of system
  set_current_thread_id(std::numeric_limits<uint32_t>::max());
  REQUIRE(current_thread_id() == system_id);

  // TODO(bwrsandman): Test on Thread object
}

TEST_CASE("Set and Test Current Thread Name", "Thread") {
  std::string new_thread_name = "Threading Test";
  set_name(new_thread_name);
}

TEST_CASE("Create and Run Thread", "Thread") {
  using namespace std::chrono_literals;

  std::unique_ptr<threading::Thread> thread;
  threading::WaitResult result;
  threading::Thread::CreationParameters params = {};
  auto func = [] { threading::Sleep(20ms); };

  // Create most basic case of thread
  thread = threading::Thread::Create(params, func);
  REQUIRE(thread->native_handle() != nullptr);
  REQUIRE(thread->affinity_mask() == 0);
  REQUIRE(thread->name() == std::string());
  result = threading::Wait(thread.get(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kSuccess);

  /*// Add thread name
  std::string new_name = "Test thread name";
  thread = threading::Thread::Create(params, func);
  REQUIRE(thread->name() == std::string());
  thread->set_name(new_name);
  REQUIRE(thread->name() == new_name);
  result = threading::Wait(thread.get(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kSuccess);

  // Use Terminate to end an inifinitely looping thread
  thread = threading::Thread::Create(params, [] {
    while (true) {
    }
  });
  result = threading::Wait(thread.get(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kTimeout);
  thread->Terminate(-1);
  result = threading::Wait(thread.get(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kSuccess);

  // Call Exit from inside an inifinitely looping thread
  thread = threading::Thread::Create(params, [] {
    while (true) {
      threading::Thread::Exit(-1);
    }
  });
  result = threading::Wait(thread.get(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kSuccess);

  // Add an Exit command with QueueUserCallback
  bool is_modified = false;
  thread = threading::Thread::Create(params, [] {
    // Using Alertable so callback is registered
    threading::AlertableSleep(90ms);
  });
  result = threading::Wait(thread.get(), true, 50ms);
  REQUIRE(result == threading::WaitResult::kTimeout);
  REQUIRE(!is_modified);
  thread->QueueUserCallback([&is_modified] {
    is_modified = true;
    threading::Thread::Exit(0);
  });
  result = threading::Wait(thread.get(), true, 100ms);
  REQUIRE(result != threading::WaitResult::kTimeout);
  REQUIRE(result != threading::WaitResult::kFailed);
  // TODO(bwrsandman): Should be kUserCallback?
  REQUIRE(is_modified);

  // Call wait on self
  result = threading::Wait(threading::Thread::GetCurrentThread(), false, 50ms);
  REQUIRE(result == threading::WaitResult::kTimeout);*/

  // TODO(bwrsandman): Test suspention and resume
  // TODO(bwrsandman): Test with different priorities
  // TODO(bwrsandman): Test different stack sizes
  // TODO(bwrsandman): Test setting and getting affinity
  // TODO(bwrsandman): Test for alerted state
}

}  // namespace test
}  // namespace base
}  // namespace xe
