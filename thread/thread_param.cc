#include <string>
#include <thread>
#include <iostream>
#include <unistd.h>

class test{
public:
    test(){
        std::cout<<"test()"<<std::endl;
    }
    test(test &t){
        std::cout<<"test(const test &t)"<<std::endl;
    }
};


void func(const test & str){
    sleep(20);
}

int main(){
    test temp;
    std::thread thread_(func, std::ref(temp));
    thread_.join();
    return 0;
}