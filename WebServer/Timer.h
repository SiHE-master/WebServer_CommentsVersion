// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <unistd.h>
#include <deque>
#include <memory>
#include <queue>
#include "HttpData.h"
#include "base/MutexLock.h"
#include "base/noncopyable.h"


class HttpData;

class TimerNode {
 public:
  TimerNode(std::shared_ptr<HttpData> requestData, int timeout);
  //析构时关闭请求数据  SPHttpData->handleClose()
  ~TimerNode();
  TimerNode(TimerNode &tn);
  //更新定时器的到点为当前时间+timeout
  void update(int timeout);
  //判断定时器是否合法
  bool isValid();
  //清除请求数据，这里只是把HttpData指针的值置空了 并置deleted_为true，表示延迟删除
  void clearReq();
  //deleted_标志置true
  void setDeleted() { deleted_ = true; }
  //获取deleted_状态
  bool isDeleted() const { return deleted_; }
  //获取到点时间
  size_t getExpTime() const { return expiredTime_; }

 private:
  bool deleted_;
  size_t expiredTime_;//单位：毫秒
  std::shared_ptr<HttpData> SPHttpData;//指向请求数据的共享指针
};

struct TimerCmp {
  bool operator()(std::shared_ptr<TimerNode> &a,
                  std::shared_ptr<TimerNode> &b) const {
    return a->getExpTime() > b->getExpTime();
  }
};

class TimerManager {
 public:
  TimerManager();
  ~TimerManager();
  //添加定时器
  void addTimer(std::shared_ptr<HttpData> SPHttpData, int timeout);
  //处理到期事件
  void handleExpiredEvent();

 private:
  typedef std::shared_ptr<TimerNode> SPTimerNode;
  std::priority_queue<SPTimerNode, std::deque<SPTimerNode>, TimerCmp>
      timerNodeQueue;//小顶堆
  // MutexLock lock;
};