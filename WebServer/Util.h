// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#pragma once
#include <cstdlib>
#include <string>

//三个重载的readn
//从fd读入最多n个字符到buff 返回读到的字符总数
ssize_t readn(int fd, void *buff, size_t n);
//从fd读入数据到inBuffer中，传出参数zero为true时表示已读到文件末尾；返回读到的字符总数
ssize_t readn(int fd, std::string &inBuffer, bool &zero);
//从fd读入数据到inBuffer中 返回读到的字符总数
ssize_t readn(int fd, std::string &inBuffer);
//将buff开头的n个字符写入fd 返回写入的字符总数
ssize_t writen(int fd, void *buff, size_t n);
//将sbuff中的数据写入fd，并清除sbuff中已写入部分； 返回写入的字符总数
ssize_t writen(int fd, std::string &sbuff);

//把一些设置打包为函数
//设置忽略捕捉到的SIGPIPE信号
void handle_for_sigpipe();
//设置fd为非阻塞 成功返回0 失败返回-1
int setSocketNonBlocking(int fd);
//禁用Nagle算法，即有数据就发，不再缓存
void setSocketNodelay(int fd);
//closesocket()关闭套接字后，让没发完的数据发送出去后在关闭socket
void setSocketNoLinger(int fd);
//关闭fd的写端
void shutDownWR(int fd);
//创建监听文件描述符，绑定IP地址和传入的端口，并开始监听
int socket_bind_listen(int port);