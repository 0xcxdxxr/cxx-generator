#pragma once

#include <optional>
#include <utility>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

namespace generator {
struct ResumeFinishedGeneratorError : std::logic_error {
  ResumeFinishedGeneratorError() :
      std::logic_error("Unable to resume a finished generator") {
  }
};

namespace __ {
struct _ {
};
}  // namespace __

template<
    typename Yield,
    typename Resume,
    typename... Args>
struct Generate;

template<
    typename Yield,
    typename Resume>
struct Ctx {
  template<
      typename Yield_,
      typename Resume_,
      typename... Args_>
  friend struct Gen;

private:
  bool done = false;
  bool yielded = true;
  std::optional <Yield> yieldVal;
  std::optional <Resume> resumeVal;
  std::mutex lock;
  std::condition_variable cv;

public:
  Resume operator=(Yield &&val) {
    return yield(std::move(val));
  }
  Resume operator=(const Yield &val) {
    return yield(val);
  }

public:
  Resume yield(Yield &&val) {
    yieldVal = std::move(val);
    return do_yield();
  }
  Resume yield(const Yield &val) {
    yieldVal = val;
    return do_yield();
  }

private:
  Resume do_yield() {
    {
      std::lock_guard <std::mutex> _(lock);
      yielded = true;
    }
    cv.notify_one();
    {
      std::unique_lock <std::mutex> _(lock);
      if (!done)
        cv.wait(_, [this] { return !yielded || done; });
    }
    if (done) {
      throw __::_();
    }

    return std::move(resumeVal).value();
  }
};

template<
    typename Yield,
    typename Resume,
    typename... Args>
struct Gen {
  using CtxType = Ctx<Yield, Resume>;
  using BodyType = std::function<Yield(
      CtxType &,
      Args...)>;

public:
  Generate(const BodyType &body, Args... args) {
    std::mutex mut;
    std::unique_lock <std::mutex> _1(mut);
    std::condition_variable cv1;
    _ = std::thread(
        [this, &cv1, &mut, body, args...] {
          {
            std::lock_guard <std::mutex> _(mut);
          }
          cv1.notify_one();
          {
            {
              std::unique_lock <std::mutex> _(ctx.lock);
              ctx.cv.wait(_, [this] { return !ctx.yielded || ctx.done; });
            }
            try {
              if (!ctx.done) {
                auto ret = body(ctx, args...);
                ctx.done = true;
                ctx.yield(ret);
              }
            }
            catch (__::_) {
            }
          }
        });
    cv1.wait(_1);
  }

  ~Generate() {
    if (!ctx.done) {
      ctx.done = true;
      ctx.lock.unlock();
      ctx.cv.notify_one();
    }
    _.value().join();
  }

public:
  Yield resume() {
    ctx.resumeVal = std::nullopt;
    return do_resume();
  }

  Yield resume(Resume &&val) {
    ctx.resumeVal = std::move(val);
    return do_resume();
  }

  Yield resume(const Resume &val) {
    ctx.resumeVal = val;
    return do_resume();
  }

  bool done() const {
    return ctx.done;
  }

private:
  Yield do_resume() {
    if (ctx.done)
      throw ResumeFinishedGeneratorError();
    {
      std::lock_guard <std::mutex> _(ctx.lock);
      ctx.yielded = false;
    }
    ctx.cv.notify_one();
    {
      std::unique_lock <std::mutex> _(ctx.lock);
      ctx.cv.wait(_, [this] { return ctx.yielded; });
    }
    return std::move(ctx.yieldVal).value();
  }

private:
  CtxType ctx;
  std::optional <std::thread> _;
};

}  // namespace generator

#ifndef CXX_GENERATOR__GENERATOR_HPP
#define yield \
    _ =

#define generator(Yield, Resume, Body)                \
    ::gen::Gen<Yield, Resume>(                          \
      [](::gen::Ctx<Yield, Resume> &_ ) -> Yield {      \
          const std::function<Yield()> body = ( Body ); \
          return body();                                \
      } )

#define CXX_GENERATOR__GENERATOR_HPP

#endif //CXX_GENERATOR__GENERATOR_HPP
