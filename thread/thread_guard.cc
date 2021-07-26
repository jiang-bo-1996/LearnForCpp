#include <thread>
#include <boost/noncopyable.hpp>

class thread_guared : boost::noncopyable
{
  std::thread &t_;
 public:
  thread_guared(std::thread &t)
    : t_(t)
  { }

  ~thread_guared(){
      if(t_.joinable()){
          t_.join();
      }
  }
};