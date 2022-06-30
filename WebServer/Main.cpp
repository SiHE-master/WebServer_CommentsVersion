// @Author Lin Ya
// @Email xxbbb@vip.qq.com
#include <getopt.h>
#include <string>
#include "EventLoop.h"
#include "Server.h"
#include "base/Logging.h"


int main(int argc, char *argv[]) {
  int threadNum = 4;//默认线程数量
  int port = 80;//默认使用端口号
  std::string logPath = "./WebServer.log"; //默认log路径

  // parse args
  int opt;
  const char *str = "t:l:p:";
  //1.单个字符，表示选项，
  //2.单个字符后接一个冒号：表示该选项后必须跟一个参数。参数紧跟在选项后或者以空格隔开。该参数的指针赋给optarg。
  //3 单个字符后跟两个冒号，表示该选项后必须跟一个参数。参数必须紧跟在选项后不能以空格隔开。该参数的指针赋给optarg。（这个特性是GNU的扩张）。
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
  Logger::setLogFileName(logPath); //手动设置log文件名
// STL库在多线程上应用
#ifndef _PTHREADS
  LOG << "_PTHREADS is not defined !"; //写入日志文件
#endif
  EventLoop mainLoop; //主事件循环
  Server myHTTPServer(&mainLoop, threadNum, port);//创建服务器
  myHTTPServer.start();//启动服务器
  mainLoop.loop();//启动mainloop
  return 0;
}
