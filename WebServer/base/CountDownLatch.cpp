// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
    : mutex_(), condition_(mutex_), count_(count) {}

void CountDownLatch::wait() {
  MutexLockGuard lock(mutex_);//无需手动解锁，函数返回时自动解锁
  while (count_ > 0) condition_.wait();
}

void CountDownLatch::countDown() {
  MutexLockGuard lock(mutex_);
  --count_;
  if (count_ == 0) condition_.notifyAll();
}