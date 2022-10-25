#include<iostream>
#include "logger.h"
using namespace std;
using namespace utility;

int main()
{
    Logger::instence()->open("./text.log");
    cout << "log" << endl;
    //Logger::instence()->set_level(Logger::WARN);
    Logger::instence()->set_max(200);
    //__FILE__用以指示本行语句所在源文件的文件名,__LINE__用以指示本行语句在源文件中的位置信息
    //Logger::instence()->log(Logger::DEBUG, __FILE__, __LINE__, "log");
    debug("debug messages");
    info("info messages");
    warn("warn messages");
    error("error messages");
    fatal("fatal messages");
    return 0;
}
