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

void Server::start() {
  eventLoopThreadPool_->start();//启动线程池
  // acceptChannel_->setEvents(EPOLLIN | EPOLLET | EPOLLONESHOT);
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);//注册读事件和边沿触发
  acceptChannel_->setReadHandler(bind(&Server::handNewConn, this));
  acceptChannel_->setConnHandler(bind(&Server::handThisConn, this));
  loop_->addToPoller(acceptChannel_, 0);//把负责监听的channel中相应事件和文件描述符注册到mainloop中poller的epollfd 并且没有设置定时器
  started_ = true;
}
//
void Server::handNewConn() {
  struct sockaddr_in client_addr;
  memset(&client_addr, 0, sizeof(struct sockaddr_in));//也可以直接sizeof client_addr
  socklen_t client_addr_len = sizeof(client_addr);
  int accept_fd = 0;
  while ((accept_fd = accept(listenFd_, (struct sockaddr *)&client_addr,
                             &client_addr_len)) > 0) {
    EventLoop *loop = eventLoopThreadPool_->getNextLoop();
    LOG << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":"
        << ntohs(client_addr.sin_port);
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
      continue;
    }
    // 设为非阻塞模式
    if (setSocketNonBlocking(accept_fd) < 0) {
      LOG << "Set non block failed!";
      // perror("Set non block failed!");
      return;
    }

    setSocketNodelay(accept_fd);
    // setSocketNoLinger(accept_fd);

    shared_ptr<HttpData> req_info(new HttpData(loop, accept_fd));
    req_info->getChannel()->setHolder(req_info);
    loop->queueInLoop(std::bind(&HttpData::newEvent, req_info));
  }
  acceptChannel_->setEvents(EPOLLIN | EPOLLET);
}