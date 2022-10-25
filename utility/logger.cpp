#include "logger.h"
#include <time.h>
#include <string.h>
#include <stdexcept>
#include <stdarg.h>
#include <iostream>
using namespace utility;

//类外静态成员初始化
const char * Logger::_level[LEVEL_COUNT] = {
    "DEBUG",
    "INFO",
    "WARN",
    "ERROR",
    "FATAL"
};

//类外初始化静态成员变量
Logger *Logger::_instence = NULL;

//初始化单例锁
pthread_once_t Logger::_once = PTHREAD_ONCE_INIT;


Logger::Logger() :
    _lev(DEBUG),
    _max(0),
    _len(0)
{
    //初始化读写锁
    pthread_rwlock_init(&_lock, NULL);
}

Logger::~Logger()
{
    pthread_rwlock_destroy(&_lock);
    close();
}

void Logger::open(const std::string & filename)
{
    _filename = filename;
    _fout.open(filename, std::ios::app);
    pthread_rwlock_rdlock(&_lock);
    if(_fout.fail())
    {
        throw std::logic_error("open file failed" + filename);
    }
    
    //获取当前文件长度
    _fout.seekp(0, std::ios::end);
    _len = _fout.tellp();
    pthread_rwlock_unlock(&_lock);

}

void Logger::close()
{
    _fout.close();
}


void Logger::log(Level level, const char * file, int line, const char * format, ...)
{
    //如果该事件的日志级别小于设置的日志级别，不记录
    if(level < _lev)
    {
        return;
    }
    if(_fout.fail())
    {
        throw std::logic_error("open file failed" + _filename);
    }

    time_t ticks = time(NULL);
/*  struct tm {
        int tm_sec;    // Seconds (0-60) 
        int tm_min;    // Minutes (0-59) 
        int tm_hour;   // Hours (0-23) 
        int tm_mday;   // Day of the month (1-31) 
        int tm_mon;    // Month (0-11) 
        int tm_year;   // Year - 1900 
        int tm_wday;   // Day of the week (0-6, Sunday = 0) 
        int tm_yday;   // Day in the year (0-365, 1 Jan = 0) 
        int tm_isdst;  // Daylight saving time
    };*/
    //获取本地时间
    struct tm * ptm  = localtime(&ticks);
    //四个字节为一个单位
    char t_buf[64];
    memset(t_buf, 0, sizeof(t_buf));
    //将ptm指向的结构格式化写入到t_buf中
    strftime(t_buf, sizeof(t_buf), "%Y-%m-%d %H:%M:%S", ptm);
    const char * fmt = "%s %s %s:%d "; //时间 级别 文件名：行号
    //获取格式化字符串长度
    int size = snprintf(NULL, 0, fmt, t_buf, _level[level], file, line);
    //printf("size = %d\n", size);
    pthread_rwlock_wrlock(&_lock);
    if(size > 0)
    {
        char * fmt_buf = new char[size + 1];
        snprintf(fmt_buf, size + 1, fmt, t_buf, _level[level], file, line);
        fmt_buf[size] = 0;
        //std::cout << fmt_buf << std::endl;
        //写入到log文件
        _fout << fmt_buf;
        _len += size;
        delete[] fmt_buf;
    }
 
    //可变参数列表
    va_list arg_list;
    va_start(arg_list, format);
    //将可变参数格式化写入到buf中， 返回值为最终生成字符串的长度
    size = vsnprintf(NULL, 0, format, arg_list);
    va_end(arg_list);
    if(size > 0)
    {
        char * vsn_buf = new char[size + 1];
        va_start(arg_list, format);
        //将可变参数格式化写入到buf中， 返回值为最终生成字符串的长度
        size = vsnprintf(vsn_buf, size + 1, format, arg_list);
        va_end(arg_list);
        _fout << vsn_buf;
        _len += size;
        //std::cout << vsn_buf << std::endl;
        _len += size;
        delete[] vsn_buf;
    }
    _fout << "\n";
    //刷新缓存
    _fout.flush();
    pthread_rwlock_unlock(&_lock);
#if 0
    std::cout << t_buf << std::endl;
    std::cout << file << std::endl;
    std::cout << line << std::endl;
    std::cout << format << std::endl;
#endif
    //超出文件大小限制
    if(_max > 0 && _len > _max)
    {
        rotate();
    }

}

void Logger::rotate()
{
    //关闭当前文件描述符
    close();

    time_t ticks = time(NULL);
    struct tm * _tm = localtime(&ticks);
    char t_buf[64];
    memset(t_buf, 0, sizeof(t_buf));
    //获取当前时间戳
    strftime(t_buf, sizeof(t_buf), ".%Y-%m-%d_%H-%M-%S", _tm);
    std::string filename = _filename + t_buf;
    //将日志文件以当前时间重命名备份
    if(rename(_filename.c_str(), filename.c_str()) != 0)
    {
        //抛出异常， strerror(errno) 获取错误码
        throw std::logic_error("rename log file error: " + std::string(strerror(errno)));
    }
    //打开新文件
    open(_filename);
    
}
