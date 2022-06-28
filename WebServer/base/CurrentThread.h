// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <stdint.h>

namespace CurrentThread {
// internal
//凡是带有__thread的变量，每个线程都拥有该变量的一份拷贝，且互不干扰。
//线程局部存储中的变量将一直存在，直至线程终止，当线程终止时会自动释放这一存储。
extern __thread int t_cachedTid;
extern __thread char t_tidString[32];
extern __thread int t_tidStringLength;
extern __thread const char* t_threadName;
void cacheTid();
//这里__builtin_expect表示t_cachedTid大概率不是0，也就是t_cachedTid!=0
inline int tid() {
  if (__builtin_expect(t_cachedTid == 0, 0)) {
    cacheTid();
  }
  return t_cachedTid;
}

inline const char* tidString()  // for logging
{
  return t_tidString;
}

inline int tidStringLength()  // for logging
{
  return t_tidStringLength;
}

inline const char* name() { return t_threadName; }
}
