#include <memory>
#include <iostream>
#include <vector>
#include <string>
#include <thread>
/*std::vector<const std::shared_ptr<std::string>> &nums_;
void bar(std::shared_ptr<std::string> param){
    std::cout << "bar " << *(param.get()) << " param.count() :" << param.use_count() << std::endl;
    std::shared_ptr<std::string> str_ = std::move(param);//拷贝
    std::cout << "*str_ = " << *str_ << "str_.use_count() :" << str_.use_count() <<std::endl;
}

void func(const std::shared_ptr<std::string> &str){
   std::cout<< "ptr.use_count(): " << str.use_count()<< std::endl;
   bar(std::move(str));
   std::cout<< "ptr.use_count(): " << str.use_count()<< std::endl;
}
*/

void func(int i,const std::string &s){
  std::cout << s << std::endl;
}

int main(){
  /*
  std::shared_ptr<std::string> ptr(std::make_shared<std::string>("hello world"));
  func(std::move(ptr));//还是引用
  std::cout<< "ptr.use_count(): " << ptr.use_count()<< std::endl;
  */
  std::string str = "124";
  std::cout << &str << std::endl;
  std::thread t(func, 1, str);
  t.join();
  return 0;
}

