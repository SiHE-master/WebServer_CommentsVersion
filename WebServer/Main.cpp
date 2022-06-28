// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include <getopt.h>
#include <string>
#include "EventLoop.h"
#include "Server.h"
#include "base/Logging.h"


int main(int argc, char *argv[]) {
  int threadNum = 4;
  int port = 80;
  std::string logPath = "./WebServer.log"; //默认log路径

  // parse args
  int opt;
  const char *str = "t:l:p:";
  while ((opt = getopt(argc, argv, str)) != -1) { //在命令行选项参数再也检查不到optstring中包含的选项时，返回－1
    switch (opt) {
      case 't': { // -t传递的参数为线程数量
        threadNum = atoi(optarg);
        break;
      }
      case 'l': {
        logPath = optarg; // -l传递的参数为log路径
        if (logPath.size() < 2 || optarg[0] != '/') { //路径无效
          printf("logPath should start with \"/\"\n");
          abort();
        }
        break;
      }
      case 'p': { //-p传递的参数为端口号
        port = atoi(optarg); 
        break;
      }
      default:
        break;
    }
  }
  Logger::setLogFileName(logPath); //设置log文件名
// STL库在多线程上应用
#ifndef _PTHREADS
  LOG << "_PTHREADS is not defined !"; //写入日志文件
#endif
  EventLoop mainLoop; //主事件循环
  Server myHTTPServer(&mainLoop, threadNum, port);
  myHTTPServer.start();
  mainLoop.loop();
  return 0;
}
