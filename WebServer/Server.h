// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <memory>
#include "Channel.h"
#include "EventLoop.h"
#include "EventLoopThreadPool.h"

class Server {
 public:
  Server(EventLoop *loop, int threadNum, int port);
  ~Server() {}
  EventLoop *getLoop() const { return loop_; } //不改变数据成员的函数加上const关键字进行标识，提高程序的可读性和可靠性。
  void start(); //启动服务器
  void handNewConn();
  void handThisConn() { loop_->updatePoller(acceptChannel_); }

 private:
  EventLoop *loop_;
  int threadNum_;
  std::unique_ptr<EventLoopThreadPool> eventLoopThreadPool_; //线程池
  bool started_; //启动状态
  std::shared_ptr<Channel> acceptChannel_;
  int port_;
  int listenFd_; //监听端口
  static const int MAXFDS = 100000;
};