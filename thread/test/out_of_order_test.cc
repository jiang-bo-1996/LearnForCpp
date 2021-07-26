#include <thread>
#include <iostream>

int x, y;

void work1(){
  x = 1;
  if(y == 0){
    std::cout<<"thread1"<<std::endl;
  }
}

void work2(){
  y = 1;
  if(x == 0){
    std::cout<<"thread2"<<std::endl;
  }
}

int main(){
  std::thread thread1(work1);
  std::thread thread2(work2);
  thread1.join();
  thread2.join();
  return 0;
}