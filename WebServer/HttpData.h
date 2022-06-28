// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <sys/epoll.h>
#include <unistd.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include "Timer.h"


class EventLoop;
class TimerNode;
class Channel;

enum ProcessState {
  STATE_PARSE_URI = 1,
  STATE_PARSE_HEADERS,
  STATE_RECV_BODY,//接收请求体
  STATE_ANALYSIS,//分析请求体
  STATE_FINISH
};

enum URIState {
  PARSE_URI_AGAIN = 1,
  PARSE_URI_ERROR,
  PARSE_URI_SUCCESS,
};

enum HeaderState {
  PARSE_HEADER_SUCCESS = 1,
  PARSE_HEADER_AGAIN,
  PARSE_HEADER_ERROR
};

enum AnalysisState { ANALYSIS_SUCCESS = 1, ANALYSIS_ERROR };

enum ParseState {
  H_START = 0,
  H_KEY,
  H_COLON,//冒号
  H_SPACES_AFTER_COLON,//冒号后空格
  H_VALUE,
  H_CR,//回车
  H_LF,//换行
  H_END_CR,//结束时回车
  H_END_LF//结束时换行
};

enum ConnectionState { H_CONNECTED = 0, H_DISCONNECTING, H_DISCONNECTED };

enum HttpMethod { METHOD_POST = 1, METHOD_GET, METHOD_HEAD };

enum HttpVersion { HTTP_10 = 1, HTTP_11 };

class MimeType {
 private:
  //getMime中调用，只执行一次
  static void init();
  static std::unordered_map<std::string, std::string> mime;
  MimeType();
  MimeType(const MimeType &m);

 public:
  //获取Multipurpose Internet Mail Extensions（MIME）
  static std::string getMime(const std::string &suffix);

 private:
  static pthread_once_t once_control;
};

class HttpData : public std::enable_shared_from_this<HttpData> {
 public:
  HttpData(EventLoop *loop, int connfd);
  //析构时关闭文件描述符
  ~HttpData() { close(fd_); }
  //复位
  void reset();
  void seperateTimer();
  //给定时器指针赋值
  void linkTimer(std::shared_ptr<TimerNode> mtimer) {
    // shared_ptr重载了bool, 但weak_ptr没有
    timer_ = mtimer;
  }
  std::shared_ptr<Channel> getChannel() { return channel_; }
  EventLoop *getLoop() { return loop_; }
  void handleClose();
  void newEvent();

 private:
  EventLoop *loop_;
  std::shared_ptr<Channel> channel_;//指向用于处理http数据的channel
  int fd_;
  std::string inBuffer_;//存储读入数据的Buffer
  std::string outBuffer_;
  bool error_;//表示是否发生错误
  ConnectionState connectionState_;//已连接，断开中，断开连接

  HttpMethod method_;//post，get，head
  HttpVersion HTTPVersion_;//两个http版本1.0和1.1
  std::string fileName_;
  std::string path_;
  int nowReadPos_;//解析时使用的下标，表示当前开始读取的位置
  ProcessState state_;//状态机，表示在处理哪一部分
  ParseState hState_;//解析请求头的状态
  bool keepAlive_;//是否保持连接
  std::map<std::string, std::string> headers_;//存储读取到的请求头信息
  std::weak_ptr<TimerNode> timer_;//定时器节点指针

  void handleRead();
  void handleWrite();
  void handleConn();
  void handleError(int fd, int err_num, std::string short_msg);
  URIState parseURI();
  HeaderState parseHeaders();
  AnalysisState analysisRequest();
};