// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <sys/epoll.h>
#include <sys/epoll.h>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include "Timer.h"

class EventLoop;
class HttpData;

class Channel {
 private:
  typedef std::function<void()> CallBack;
  EventLoop *loop_;//一个channel对应一个EventLoop
  int fd_;//一个channel对应一个文件描述符
  __uint32_t events_;// 注册fd感兴趣的事件
  __uint32_t revents_;// Poller返回的具体发生的事件
  __uint32_t lastEvents_;

  // 方便找到上层持有该Channel的对象
  std::weak_ptr<HttpData> holder_;
  //weak_ptr并没有重载 operator->和 operator *操作符，
  //因此不可直接通过weak_ptr使用对象，典型的用法是调用其lock函数来
  //获得shared_ptr示例，进而访问原始对象。
 private:
 //解析URL
  int parse_URI();
  //解析头
  int parse_Headers();
  //解析请求
  int analysisRequest();

  CallBack readHandler_;//处理读事件
  CallBack writeHandler_;//处理写事件
  CallBack errorHandler_;//处理错误事件
  CallBack connHandler_;//处理连接事件

 public:
  Channel(EventLoop *loop);
  Channel(EventLoop *loop, int fd);
  ~Channel();
  //获取绑定的文件描述符
  int getFd();
  //设置绑定的文件描述符
  void setFd(int fd);

  //设置weak_ptr指向上层持有该Channel的对象
  void setHolder(std::shared_ptr<HttpData> holder) { holder_ = holder; }
  //获取weak_ptr指向的上层持有该Channel的对象
  std::shared_ptr<HttpData> getHolder() {
    std::shared_ptr<HttpData> ret(holder_.lock());
    return ret;
  }

  //设置处理读事件
  void setReadHandler(CallBack &&readHandler) { readHandler_ = readHandler; }
  //设置处理写事件
  void setWriteHandler(CallBack &&writeHandler) {
    writeHandler_ = writeHandler;
  }
  //设置处理错误事件
  void setErrorHandler(CallBack &&errorHandler) {
    errorHandler_ = errorHandler;
  }
  //设置处理连接事件
  void setConnHandler(CallBack &&connHandler) { connHandler_ = connHandler; }

  //调用处理事件
  void handleEvents() {
    events_ = 0;
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {//EPOLLHUP只会在写的时候发生
      events_ = 0;
      return;
    }
    if (revents_ & EPOLLERR) {//有错误发生 处理错误
      if (errorHandler_) errorHandler_();
      events_ = 0;
      return;
    }
    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {//对于EPOLLRDHUP，read会返回0
      handleRead();
    }
    if (revents_ & EPOLLOUT) {
      handleWrite();
    }
    handleConn();//最后必然会调用处理连接事件
  }
  void handleRead();
  void handleWrite();
  void handleError(int fd, int err_num, std::string short_msg);//明明没实现，点击却跳到HttpData的同名函数去了
  void handleConn();

  //设置具体发生的事件 感觉这里muduo的写得更好些
  void setRevents(__uint32_t ev) { revents_ = ev; }

  //注册fd感兴趣的事件
  void setEvents(__uint32_t ev) { events_ = ev; }
  //返回fd感兴趣的事件
  __uint32_t &getEvents() { return events_; }

  //更新lastEvents_，并返回是否已更新
  bool EqualAndUpdateLastEvents() {
    bool ret = (lastEvents_ == events_);
    lastEvents_ = events_;
    return ret;
  }

  //获取lastEvents_
  __uint32_t getLastEvents() { return lastEvents_; }
};

typedef std::shared_ptr<Channel> SP_Channel;