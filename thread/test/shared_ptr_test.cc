#define _CRT_SECURE_NO_WARNINGS 1
#include <iostream>
#include <stdio.h>
struct Fclose
{
  void operator()(FILE *&fp){
    std::cout<<"Fclose()"<<std::endl;
    ::fclose(fp);
  }
};

template <class T>
struct Free
{
  void operator()(T *&p){
    free(p);
    p = NULL;
  }
};


template <class T>
class Delete
{
 public:
  void operator()(T *&p){
    delete p;
    p = NULL;
  }
};
template <class T, class Destory = Delete<T>>
class SharedPtr{
 public:
  SharedPtr(T *ptr = 0)
    : _ptr(ptr),
      _pCount(NULL)
  {
    if(_ptr){
      _pCount = new int(1);
    }  
  }

  ~SharedPtr(){
    Release();
  }

  SharedPtr(const SharedPtr<T> &sp)
    : _ptr(sp._ptr),
      _pCount(sp._pCount)
  {
    if(_ptr){
      ++ GetRef();
    }
  }

  SharedPtr<T> &operator=(const SharedPtr<T> &sp){
    if(this != &sp){
      Release();
      _ptr = sp._ptr;
      _pCount = sp._pCount;
      ++ GetRef();
    }
    return *this;
  }

  void Release(){
    if(_ptr && -- GetRef() == 0){
      Destory()(_ptr);
      delete _pCount;
      _pCount =NULL;
    }
  }

  int &GetRef(){
    return *_pCount;
  }
 private:
  T* _ptr;
  int *_pCount;
};
