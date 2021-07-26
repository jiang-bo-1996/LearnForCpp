#pragma once
#include "Thread.h"
#include <sys/acct.h>

#include <boost/noncopyable.hpp>
#include <assert.h>
#include <pthread.h>

namespace jiangbo{
class MutexLock: boost::noncopyable{
public:
    MutexLock()
        : holder_(0){
        pthread_mutex_init(&mutex_, NULL);
    }

    ~MutexLock(){
        assert(holder_ == 0);//向stderr打印一条出错信息，调用abort()来终止程序运行
        pthread_mutex_destroy(&mutex_);
    }

    bool isLockedByThisThread(){
        return holder_ == CurrentThread::tid();
    }

    bool assertLocked(){
        assert(isLockedByThisThread());
    }

    void lock(){
        pthread_mutex_lock(&mutex_);
        holder_ = CurrentThread::tid();
    }

    void unlock(){
        holder_ = 0;
        pthread_mutex_unlock(&mutex_);
    }
private:

    pthread_mutex_t mutex_;
    pid_t holder_;//通过是否存在锁主人来判断是否加锁
};

class MutexLockGuard : boost::noncopyable{
public:
    explicit MutexLockGuard(MutexLock& Mutex):mutex_(mutex_){
        mutex_.lock();
    }
    ~MutexLockGuard(){
        mutex_.unlock();
    }
private:

    MutexLock& mutex_;
};
}

#define MutexLockGuard(x) error "Missing guard object name"