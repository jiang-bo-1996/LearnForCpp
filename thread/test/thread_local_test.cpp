#include <thread>
#include <iostream>
class local_{
public:
    local_(){
        std::cout<<"local()"<<std::endl;
    }
    void show(){
        std::cout<<"show()"<<std::endl;
    }
    int a() const {return a_;}
    int a_;
};
thread_local local_ Temp;

void Threadworker(){
    std::cout<<"subthread: "<<pthread_self()<<std::endl;
    Temp.show();
    std::cout<<"subthread: "<<Temp.a()<<std::endl;
}

int main(){
    std::cout<<"main: "<<pthread_self()<<std::endl;
    std::thread thread_(Threadworker);
    Temp.show();
    std::cout<<"subthread: "<<Temp.a()<<std::endl;
    thread_.join();
    return 0;
}