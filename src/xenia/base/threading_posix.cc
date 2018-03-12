/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2014 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#include "xenia/base/threading.h"

#include "xenia/base/assert.h"
#include "xenia/base/logging.h"

#include <pthread.h>
#include <signal.h>
#include <sys/eventfd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctime>

namespace xe {
namespace threading {

template <typename _Rep, typename _Period>
timespec DurationToTimeSpec(std::chrono::duration<_Rep, _Period> duration) {
  auto nanoseconds =
      std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
  auto div = ldiv(nanoseconds.count(), 1000000000L);
  return timespec{div.quot, div.rem};
}

// Thread interruption is done using user-defined signals
// This implementation uses the SIGRTMAX - SIGRTMIN to signal to a thread
// gdb tip, for SIG = SIGRTMIN + SignalType : handle SIG nostop
enum class SignalType { kHighResolutionTimer, k_Count };

int GetSystemSignal(SignalType num) {
  auto result = SIGRTMIN + static_cast<int>(num);
  assert_true(result < SIGRTMAX);
  return result;
}

SignalType GetSystemSignalType(int num) {
  return static_cast<SignalType>(num - SIGRTMIN);
}

thread_local std::array<bool, static_cast<size_t>(SignalType::k_Count)>
    signal_handler_installed = {};

void signal_handler(int signal, siginfo_t* info, void* /*context*/) {
  switch (GetSystemSignalType(signal)) {
    case SignalType::kHighResolutionTimer: {
      assert_true(info->si_value.sival_ptr != nullptr);
      auto callback =
          *static_cast<std::function<void()>*>(info->si_value.sival_ptr);
      callback();
    } break;
    default:
      assert_always();
  }
}

void install_signal_handler(SignalType type) {
  if (signal_handler_installed[static_cast<size_t>(type)]) return;
  struct sigaction action;
  action.sa_flags = SA_SIGINFO;
  action.sa_sigaction = signal_handler;
  sigemptyset(&action.sa_mask);
  if (sigaction(GetSystemSignal(type), &action, nullptr) == -1)
    signal_handler_installed[static_cast<size_t>(type)] = true;
}

// TODO(dougvj)
void EnableAffinityConfiguration() {}

// uint64_t ticks() { return mach_absolute_time(); }

uint32_t current_thread_system_id() {
  return static_cast<uint32_t>(syscall(SYS_gettid));
}

void set_name(const std::string& name) {
  pthread_setname_np(pthread_self(), name.c_str());
}

void set_name(std::thread::native_handle_type handle, const std::string& name) {
  pthread_setname_np(handle, name.c_str());
}

void MaybeYield() {
  pthread_yield();
  __sync_synchronize();
}

void SyncMemory() { __sync_synchronize(); }

void Sleep(std::chrono::microseconds duration) {
  timespec rqtp = DurationToTimeSpec(duration);
  timespec rmtp = {};
  auto p_rqtp = &rqtp;
  auto p_rmtp = &rmtp;
  int ret = 0;
  do {
    ret = nanosleep(p_rqtp, p_rmtp);
    // Swap requested for remaining in case of signal interruption
    // in which case, we start sleeping again for the remainder
    std::swap(p_rqtp, p_rmtp);
  } while (ret == -1 && errno == EINTR);
}

// TODO(dougvj) Not sure how to implement the equivalent of this on POSIX.
SleepResult AlertableSleep(std::chrono::microseconds duration) {
  sleep(duration.count() / 1000);
  return SleepResult::kSuccess;
}

// TODO(dougvj) We can probably wrap this with pthread_key_t but the type of
// TlsHandle probably needs to be refactored
TlsHandle AllocateTlsHandle() {
  assert_always();
  return 0;
}

bool FreeTlsHandle(TlsHandle handle) { return true; }

uintptr_t GetTlsValue(TlsHandle handle) {
  assert_always();
  return 0;
}

bool SetTlsValue(TlsHandle handle, uintptr_t value) {
  assert_always();
  return false;
}

// TODO(dougvj)
class PosixHighResolutionTimer : public HighResolutionTimer {
 public:
  PosixHighResolutionTimer(std::function<void()> callback)
      : callback_(callback), timer_(nullptr) {}
  ~PosixHighResolutionTimer() override {
    if (timer_) timer_delete(timer_);
  }

  bool Initialize(std::chrono::milliseconds period) {
    // Create timer
    sigevent sev{};
    itimerspec its{};
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = GetSystemSignal(SignalType::kHighResolutionTimer);
    sev.sigev_value.sival_ptr = (void*)&callback_;
    if (timer_create(CLOCK_REALTIME, &sev, &timer_) == -1) return false;

    // Start timer
    auto period_ns =
        std::chrono::duration_cast<std::chrono::nanoseconds>(period).count();
    its.it_value.tv_sec = period_ns / 1000000000;
    its.it_value.tv_nsec = period_ns % 1000000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
    return timer_settime(timer_, 0, &its, nullptr) != -1;
  }

 private:
  std::function<void()> callback_;
  timer_t timer_;
};

std::unique_ptr<HighResolutionTimer> HighResolutionTimer::CreateRepeating(
    std::chrono::milliseconds period, std::function<void()> callback) {
  install_signal_handler(SignalType::kHighResolutionTimer);
  auto timer = std::make_unique<PosixHighResolutionTimer>(std::move(callback));
  if (!timer->Initialize(period)) {
    return nullptr;
  }
  return std::unique_ptr<HighResolutionTimer>(timer.release());
}

// TODO(dougvj) There really is no native POSIX handle for a single wait/signal
// construct pthreads is at a lower level with more handles for such a mechanism
// This simple wrapper class could function as our handle, but probably needs
// some more functionality
class PosixCondition {
 public:
  PosixCondition() : signal_(false) {
    pthread_mutex_init(&mutex_, NULL);
    pthread_cond_init(&cond_, NULL);
  }

  ~PosixCondition() {
    pthread_mutex_destroy(&mutex_);
    pthread_cond_destroy(&cond_);
  }

  void Signal() {
    pthread_mutex_lock(&mutex_);
    signal_ = true;
    pthread_cond_broadcast(&cond_);
    pthread_mutex_unlock(&mutex_);
  }

  void Reset() {
    pthread_mutex_lock(&mutex_);
    signal_ = false;
    pthread_mutex_unlock(&mutex_);
  }

  bool Wait(unsigned int timeout_ms) {
    // Assume 0 means no timeout, not instant timeout
    if (timeout_ms == 0) {
      Wait();
    }
    struct timespec time_to_wait;
    struct timeval now;
    gettimeofday(&now, NULL);

    // Add the number of seconds we want to wait to the current time
    time_to_wait.tv_sec = now.tv_sec + (timeout_ms / 1000);
    // Add the number of nanoseconds we want to wait to the current nanosecond
    // stride
    long nsec = (now.tv_usec + (timeout_ms % 1000)) * 1000;
    // If we overflowed the nanosecond count then we add a second
    time_to_wait.tv_sec += nsec / 1000000000UL;
    // We only add nanoseconds within the 1 second stride
    time_to_wait.tv_nsec = nsec % 1000000000UL;
    pthread_mutex_lock(&mutex_);
    while (!signal_) {
      int status = pthread_cond_timedwait(&cond_, &mutex_, &time_to_wait);
      if (status == ETIMEDOUT) return false;  // We timed out
    }
    pthread_mutex_unlock(&mutex_);
    return true;  // We didn't time out
  }

  bool Wait() {
    pthread_mutex_lock(&mutex_);
    while (!signal_) {
      pthread_cond_wait(&cond_, &mutex_);
    }
    pthread_mutex_unlock(&mutex_);
    return true;  // Did not time out;
  }

 private:
  bool signal_;
  pthread_cond_t cond_;
  pthread_mutex_t mutex_;
};

// PosixWaitable and PosixWaitableHandle are used to define Waits according to
// different Posix natives without. In order to use the Wait function, classes
// will need to derive from PosixWaitable;
class PosixWaitHandle;

class PosixWaitHandleNativeHandle {
 public:
  explicit PosixWaitHandleNativeHandle(PosixWaitHandle* wait_handle)
      : wait_handle_(wait_handle) {}
  virtual WaitResult Wait(bool is_alertable, std::chrono::milliseconds timeout);

 private:
  PosixWaitHandle* wait_handle_;
};

class PosixWaitHandle {
 public:
  PosixWaitHandle()
      : handle_(std::make_unique<PosixWaitHandleNativeHandle>(this)) {}
  virtual WaitResult Wait(bool is_alertable,
                          std::chrono::milliseconds timeout) = 0;

 protected:
  std::unique_ptr<PosixWaitHandleNativeHandle> handle_;
};

WaitResult PosixWaitHandleNativeHandle::Wait(
    bool is_alertable, std::chrono::milliseconds timeout) {
  return wait_handle_->Wait(is_alertable, timeout);
}

// Native posix thread handle
template <typename T>
class PosixThreadHandle : public T, public PosixWaitHandle {
 public:
  explicit PosixThreadHandle(pthread_t thread) : thread_(thread) {}
  ~PosixThreadHandle() override {}

 protected:
  void* native_handle() const override {
    return reinterpret_cast<void*>(handle_.get());
  }

  pthread_t thread_;
};

// This is wraps a condition object as our handle because posix has no single
// native handle for higher level concurrency constructs such as semaphores
template <typename T>
class PosixConditionHandle : public T {
 public:
  ~PosixConditionHandle() override {}

 protected:
  void* native_handle() const override {
    return reinterpret_cast<void*>(const_cast<PosixCondition*>(&handle_));
  }

  PosixCondition handle_;
};

template <typename T>
class PosixFdHandle : public T, public PosixWaitHandle {
 public:
  explicit PosixFdHandle(intptr_t fd) : fd_(fd) {}
  ~PosixFdHandle() override {
    close(fd_);
    fd_ = 0;
  }

  WaitResult Wait(bool is_alertable,
                  std::chrono::milliseconds timeout) override {
    fd_set set;
    struct timeval time_val;
    int ret;

    FD_ZERO(&set);
    FD_SET(fd_, &set);

    time_val.tv_sec = timeout.count() / 1000;
    time_val.tv_usec = timeout.count() * 1000;
    ret = select(fd_ + 1, &set, NULL, NULL, &time_val);
    if (ret == -1) {
      return WaitResult::kFailed;
    } else if (ret == 0) {
      return WaitResult::kTimeout;
    } else {
      uint64_t buf = 0;
      ret = read(fd_, &buf, sizeof(buf));
      if (ret < 8) {
        return WaitResult::kTimeout;
      }

      return WaitResult::kSuccess;
    }
  }

 protected:
  void* native_handle() const override {
    return reinterpret_cast<void*>(handle_.get());
  }

  intptr_t fd_;
};

WaitResult Wait(WaitHandle* wait_handle, bool is_alertable,
                std::chrono::milliseconds timeout) {
  // wait_handle must be of a type deriving from PosixWaitHandle and have
  // PosixWaitHandleNativeHandle as native_handle
  assert_true(dynamic_cast<PosixWaitHandle*>(wait_handle));

  auto handle = reinterpret_cast<PosixWaitHandleNativeHandle*>(
      wait_handle->native_handle());
  if (!handle) return WaitResult::kFailed;

  return handle->Wait(is_alertable, timeout);
}

// TODO(dougvj)
WaitResult SignalAndWait(WaitHandle* wait_handle_to_signal,
                         WaitHandle* wait_handle_to_wait_on, bool is_alertable,
                         std::chrono::milliseconds timeout) {
  assert_always();
  return WaitResult::kFailed;
}

// TODO(dougvj)
std::pair<WaitResult, size_t> WaitMultiple(WaitHandle* wait_handles[],
                                           size_t wait_handle_count,
                                           bool wait_all, bool is_alertable,
                                           std::chrono::milliseconds timeout) {
  assert_always();
  return std::pair<WaitResult, size_t>(WaitResult::kFailed, 0);
}

class PosixEvent : public PosixFdHandle<Event> {
 public:
  PosixEvent(intptr_t fd) : PosixFdHandle(fd) {}
  ~PosixEvent() override = default;
  void Set() override {
    uint64_t buf = 1;
    write(fd_, &buf, sizeof(buf));
  }
  void Reset() override {
    using namespace std::chrono_literals;
    WaitResult waitResult;
    do {
      waitResult = Wait(false, 0ms);
    } while (waitResult == WaitResult::kSuccess);
  }
  // TODO(bwrsandman)
  void Pulse() override { assert_always(); }
};

std::unique_ptr<Event> Event::CreateManualResetEvent(bool initial_state) {
  // Linux's eventfd doesn't appear to support manual reset natively.
  return nullptr;
}

std::unique_ptr<Event> Event::CreateAutoResetEvent(bool initial_state) {
  int fd = eventfd(initial_state ? 1 : 0, EFD_CLOEXEC);
  if (fd == -1) {
    return nullptr;
  }

  return std::make_unique<PosixEvent>(fd);
}

// TODO(dougvj)
class PosixSemaphore : public PosixConditionHandle<Semaphore> {
 public:
  PosixSemaphore(int initial_count, int maximum_count) { assert_always(); }
  ~PosixSemaphore() override = default;
  bool Release(int release_count, int* out_previous_count) override {
    assert_always();
    return false;
  }
};

std::unique_ptr<Semaphore> Semaphore::Create(int initial_count,
                                             int maximum_count) {
  return std::make_unique<PosixSemaphore>(initial_count, maximum_count);
}

// TODO(dougvj)
class PosixMutant : public PosixConditionHandle<Mutant> {
 public:
  PosixMutant(bool initial_owner) { assert_always(); }
  ~PosixMutant() = default;
  bool Release() override {
    assert_always();
    return false;
  }
};

std::unique_ptr<Mutant> Mutant::Create(bool initial_owner) {
  return std::make_unique<PosixMutant>(initial_owner);
}

// TODO(dougvj)
class PosixTimer : public PosixConditionHandle<Timer> {
 public:
  PosixTimer(bool manual_reset) { assert_always(); }
  ~PosixTimer() = default;
  bool SetOnce(std::chrono::nanoseconds due_time,
               std::function<void()> opt_callback) override {
    assert_always();
    return false;
  }
  bool SetRepeating(std::chrono::nanoseconds due_time,
                    std::chrono::milliseconds period,
                    std::function<void()> opt_callback) override {
    assert_always();
    return false;
  }
  bool Cancel() override {
    assert_always();
    return false;
  }
};

std::unique_ptr<Timer> Timer::CreateManualResetTimer() {
  return std::make_unique<PosixTimer>(true);
}

std::unique_ptr<Timer> Timer::CreateSynchronizationTimer() {
  return std::make_unique<PosixTimer>(false);
}

class PosixThread : public PosixThreadHandle<Thread> {
 public:
  explicit PosixThread(pthread_t thread) : PosixThreadHandle(thread) {}
  PosixThread(pthread_t thread, pthread_mutex_t* wait_end_mutex)
      : PosixThreadHandle(thread), wait_end_mutex_(wait_end_mutex) {}
  ~PosixThread() {
    if (wait_end_mutex_) {
      pthread_mutex_destroy(wait_end_mutex_.get());
    }
  }

  void set_name(std::string name) override {
    pthread_setname_np(thread_, name.c_str());
  }

  uint32_t system_id() const override { return 0; }

  // TODO(DrChat)
  uint64_t affinity_mask() override { return 0; }
  void set_affinity_mask(uint64_t mask) override { assert_always(); }

  int priority() override {
    int policy;
    struct sched_param param;
    int ret = pthread_getschedparam(thread_, &policy, &param);
    if (ret != 0) {
      return -1;
    }

    return param.sched_priority;
  }

  void set_priority(int new_priority) override {
    struct sched_param param;
    param.sched_priority = new_priority;
    int ret = pthread_setschedparam(thread_, SCHED_FIFO, &param);
  }

  // TODO(DrChat)
  void QueueUserCallback(std::function<void()> callback) override {
    assert_always();
  }

  bool Resume(uint32_t* out_new_suspend_count = nullptr) override {
    assert_always();
    return false;
  }

  bool Suspend(uint32_t* out_previous_suspend_count = nullptr) override {
    assert_always();
    return false;
  }

  void Terminate(int exit_code) override {}

  WaitResult Wait(bool is_alterable,
                  std::chrono::milliseconds timeout) override {
    int ret;
    void* thread_return;
    timespec ts;

    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += (timeout.count() % 1000) * 1000000;
    if (ts.tv_nsec > 1000000000) {
      ts.tv_sec += ts.tv_nsec / 1000000000 + timeout.count() / 1000;
      ts.tv_nsec = ts.tv_nsec % 1000000000;
    }

    ret = pthread_mutex_timedlock(wait_end_mutex_.get(), &ts);
    switch (ret) {
      case 0:
        pthread_mutex_unlock(wait_end_mutex_.get());
        break;
      case EOWNERDEAD:
        return WaitResult::kAbandoned;
      case ETIMEDOUT:
        return WaitResult::kTimeout;
      default:
        return WaitResult::kFailed;
    }

    ret = pthread_join(thread_, &thread_return);
    free(thread_return);
    if (ret == 0) {
      return WaitResult::kSuccess;
    } else {
      return WaitResult::kFailed;
    }
  }

 private:
  std::unique_ptr<pthread_mutex_t> wait_end_mutex_;
};

thread_local std::unique_ptr<PosixThread> current_thread_ = nullptr;

struct ThreadStartData {
  std::function<void()> start_routine;
  pthread_mutex_t* wait_end_mutex;
  Event* started_event;
};
void* ThreadStartRoutine(void* parameter) {
  current_thread_ = std::make_unique<PosixThread>(::pthread_self());

  auto start_data = reinterpret_cast<ThreadStartData*>(parameter);
  pthread_mutex_lock(start_data->wait_end_mutex);
  start_data->started_event->Set();
  start_data->start_routine();
  pthread_mutex_unlock(start_data->wait_end_mutex);
  delete start_data;
  return nullptr;
}

std::unique_ptr<Thread> Thread::Create(CreationParameters params,
                                       std::function<void()> start_routine) {
  using namespace std::chrono_literals;
  auto wait_end_mutex = new pthread_mutex_t(PTHREAD_MUTEX_INITIALIZER);
  std::unique_ptr<Event> started_event = Event::CreateAutoResetEvent(false);
  auto start_data = new ThreadStartData(
      {std::move(start_routine), wait_end_mutex, started_event.get()});

  assert_false(params.create_suspended);
  pthread_t handle;
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  int ret = pthread_create(&handle, &attr, ThreadStartRoutine, start_data);
  if (ret != 0) {
    // TODO(benvanik): pass back?
    auto last_error = errno;
    XELOGE("Unable to pthread_create: %d", last_error);
    delete start_data;
    return nullptr;
  }
  if (!params.create_suspended) {
    Wait(started_event.get(), false, 1ms);
  }

  return std::make_unique<PosixThread>(handle, wait_end_mutex);
}

Thread* Thread::GetCurrentThread() {
  if (current_thread_) {
    return current_thread_.get();
  }

  pthread_t handle = pthread_self();

  current_thread_ = std::make_unique<PosixThread>(handle);
  return current_thread_.get();
}

void Thread::Exit(int exit_code) {
  pthread_exit(reinterpret_cast<void*>(exit_code));
}

}  // namespace threading
}  // namespace xe
