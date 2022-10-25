#pragma once
#include <string>
#include <fstream>
#include <pthread.h>
namespace utility
{

//定义宏函数，简化使用该log时的输入操作
#define debug(format, ...) \
    Logger::instence()->log(Logger::DEBUG, __FILE__, __LINE__, format, ##__VA_ARGS__);

#define info(format, ...) \
    Logger::instence()->log(Logger::INFO, __FILE__, __LINE__, format, ##__VA_ARGS__);

#define warn(format, ...) \
    Logger::instence()->log(Logger::WARN, __FILE__, __LINE__, format, ##__VA_ARGS__);

#define error(format, ...) \
    Logger::instence()->log(Logger::ERROR, __FILE__, __LINE__, format, ##__VA_ARGS__);

#define fatal(format, ...) \
    Logger::instence()->log(Logger::FATAL, __FILE__, __LINE__, format, ##__VA_ARGS__);

class Logger
{
public:
    enum Level
    {
        DEBUG = 0, //调试
        INFO,      //输出信息
        WARN,      //警告
        ERROR,     //错误
        FATAL,     //严重错误
        LEVEL_COUNT
    };

    //打开文件（记录日志内容）, filename传入参数，const不可修改
    void open(const std::string & filename);

    //关闭文件
    void close();

    //日志(写到文件里), 不定参数，日志的内容
    void log(Level level, const char * file, int line, const char * format, ...);

    //设置日志级别（低于该级别的内容不记录）
    void set_level(Level level)
    {
        _lev = level;
    }

    //设置日志最大文件限制
    void set_max(int max)
    {
        _max = max;
    }

    //创建单例的方法
    static void init()
    {
        _instence = new Logger();
    }
    //获取单例的方法
    static Logger * instence()
    {
        //保证init方法在这个进程执行过程中只执行一次
        pthread_once(&_once, init);
        return _instence;
    }

private:
    //构造私有化
    Logger();
    ~Logger();

    //日志翻滚（超过设置日志文件大小，新建日志文件记录）
    void rotate();


private:
    Level _lev; //日志级别
    int _max;   //日志最大限制
    int _len;   //当前日志长度

    std::string _filename; //文件名
    std::ofstream _fout; //文件io（文件输出流）

    pthread_rwlock_t _lock; //文件读写锁

    static const char * _level[LEVEL_COUNT]; //记录日志级别字符串
    static Logger *_instence; //单例对象
    static pthread_once_t _once; //单例锁
};
}
