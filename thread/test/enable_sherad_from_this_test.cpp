#include <stdio.h>
#include <iostream>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

class Test:public boost::enable_shared_from_this<Test>{
public:
    ~Test(){
        std::cout<<"~Test()"<<std::endl;
    };
    boost::shared_ptr<Test> GetObject(){
        return shared_from_this();
    }
};

int main(){
    boost::shared_ptr<Test> p(new Test());
    std::cout<<"q.use_count():"<<p.use_count()<<std::endl;
    boost::shared_ptr<Test> q = p->GetObject();
    std::cout<<"q.use_count():"<<p.use_count()<<std::endl;
    return 0;
}