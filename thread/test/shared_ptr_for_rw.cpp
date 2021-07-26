#include <mutex>
#include <stdio.h>
#include <set>
#include <thread>
#include <unistd.h>
#include <boost/shared_ptr.hpp>
class Request;
class Inventory
{
    typedef boost::weak_ptr<std::set<Request *>> setPtr;
public:
    Inventory()
    {
        
    }
    void add(Request *req)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        requests_.insert(req);
    }
    void remove(Request *req) __attribute__((noinline))
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if(!requestsPtr_.unique())
        {
            requestsPtr_.reset(new std::set<Request *>(*requestsPtr_));
            printf("copy the whole list!\n");
        }
        assert(requestsPtr_.unique());

    }
    void printAll();
private:
    std::mutex mutex_;
    std::set<Request *> requests_;
    setPtr requestsPtr_;
};
Inventory g_inventory;
class Request
{
public:
    void process() __attribute__((noinline))
    {
        std::unique_lock<std::mutex> lock(mutex_);
        g_inventory.add(this);
    }
    ~Request()__attribute__((noinline))
    { 
        std::unique_lock<std::mutex> lock(mutex_);
        sleep(1);
        g_inventory.remove(this);
    }
    void print() const __attribute__((noinline))
    {
        std::unique_lock<std::mutex> lock(mutex_);
    }
private:
    mutable std::mutex mutex_;
};

void Inventory::printAll()
{
    boost::shared_ptr<std::set<Request *>> requestsPtr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        requestsPtr = requestsPtr_;//提升计数，防止并发写
        assert(requestsPtr_.unique());
    }
    for(std::set<Request *>::const_iterator it = requests_.begin();
        it != requests_.end(); it ++)
    {
        (*it)->print();
    }
    printf("Inventory::printALL() unlocked\n");
}

void threadFunc()
{
    Request *req = new Request;
    req->process();
    delete req;
}
int main()
{
    printf("主线程开始!");
    std::thread thread(threadFunc);
    usleep(500 * 1000);
    g_inventory.printAll();
    thread.join();
    return 0;
}