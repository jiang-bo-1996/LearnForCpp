#include <pthread.h>
#include <iostream>
#include <thread>

pthread_once_t once_control = PTHREAD_ONCE_INIT;

void worker_1(){
    std::cout<<"worker_1() begin..."<<std::endl;
    for(; ; );
    std::cout<<"worker_1() end..."<<std::endl;
}

void work(){
    std::cout<<"work() begin..."<<std::endl;
    pthread_once(&once_control, worker_1);
    std::cout<<"work() end..."<<std::endl;
}

int main(){
    std::thread thread1_(work);
    std::thread thread2_(work);//某一个线程永远阻塞等待另一个线程结束执行pthread_once函数
    thread1_.join();
    thread2_.join();
    return 0;
}
