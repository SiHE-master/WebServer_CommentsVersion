// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include "Server.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <functional>
#include "Util.h"
#include "base/Logging.h"

Server::Server(EventLoop *loop, int threadNum, int port)
    : loop_(loop),
      threadNum_(threadNum),
      eventLoopThreadPool_(new EventLoopThreadPool(loop_, threadNum)),
      started_(false),
      acceptChannel_(new Channel(loop_)),//这里默认绑定的文件描述符是0，后面会改
      port_(port),
      listenFd_(socket_bind_listen(port_)) {
  acceptChannel_->setFd(listenFd_);//先创建后绑定
  handle_for_sigpipe();//忽略sigpipe信号 为什么要设置这个？有用到管道吗
  if (setSocketNonBlocking(listenFd_) < 0) {//使用IO多路复用API epoll时，如果设置listenfd阻塞，如果不能建立连接，会在accept处阻塞，从而阻塞整个主线程使其不能执行下去
    perror("set socket non block failed");
    abort();
  }
}
//启动线程池，设置acceptChannel_中的事件，并绑定处理新连接的函数，并注册事件
void Server::start() {
  eventLoopThreadPool_->start();//启动线程池
  // acceptChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);//注册读事件和边沿触发
  acceptChannel_->setReadHandler(bind(&Server::handNewConn, this));
  acceptChannel_->setConnHandler(bind(&Server::handThisConn, this));
  loop_->addToPoller(acceptChannel_, 0);//把负责监听的channel中相应事件和文件描述符注册到mainloop中poller的epollfd 并且没有设置定时器
  started_ = true;
}
//有新连接到来 accept
void Server::handNewConn() {
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(struct sockaddr_in));//也可以直接sizeof client_addr
  socklen_t client_addr_len = sizeof(client_addr);
  int accept_fd = 0;
  //循环accept直到没有新连接
  while ((accept_fd = accept(listenFd_, (struct sockaddr *)&client_addr,
                             &client_addr_len)) > 0) {
    EventLoop *loop = eventLoopThreadPool_->getNextLoop();//从线程池中为新连接分配一个线程
    LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
        << ntohs(client_addr.sin_port);//用client_addr.sin_addr.s_addr也可以
    // cout << "new connection" << endl;
    // cout << inet_ntoa(client_addr.sin_addr) << endl;
    // cout << ntohs(client_addr.sin_port) << endl;
    /*
    // TCP的保活机制默认是关闭的
    int optval = 0;
    socklen_t len_optval = 4;
    getsockopt(accept_fd, SOL_SOCKET,  SO_KEEPALIVE, &optval, &len_optval);
    cout << "optval ==" << optval << endl;
    */
    // 限制服务器的最大并发连接数
    if (accept_fd >= MAXFDS) {
      close(accept_fd);
      continue;//continue的目的应该是下一轮说不定有文件描述符空出来了可以重用
    }
    // 设为非阻塞模式 原因：
    //使用epoll的ET模式
    //epoll返回读写事件，但不一定真的可读写
    if (setSocketNonBlocking(accept_fd) < 0) {
      LOG << "Set non block failed!";
      // perror("Set non block failed!");
      return;
    }

    setSocketNodelay(accept_fd);
    // setSocketNoLinger(accept_fd);

    shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));//构造时初始化了channel并指向
    req_info->getChannel()->setHolder(req_info);//设置channel指向上层持有这个channel的HttpData对象
    loop->queueInLoop(std::bind(&HttpData::newEvent, req_info));//把新事件到来的设置函数加入该eventloop下的待处理事件队列
  }
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);//重新又注册的原因是getEventsRequest中清掉了
}