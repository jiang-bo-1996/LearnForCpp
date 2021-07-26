#include <list>
#include <vector>
#include <functional>
#include <utility>
#include <shared_mutex>
#include <mutex>
#include <boost/scoped_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <unordered_map>
#include <thread>
#include <iostream>

#include "../timestamp/TimeStamp.h"

template <typename Key, typename Value,typename Hash = std::hash<Key>>
class threadsafe_lookup_table
{
private:
  class bucket_type{
  private:
    typedef std::pair<Key, Value> bucket_value;
    typedef std::list<bucket_value> bucket_data;
    typedef typename bucket_data::iterator bucket_iterator;

    bucket_data data_;
    mutable std::shared_mutex mutex_;

    bucket_iterator find_entry_for(Key const &key) const{
      return std::find_if(data_.begin(), data_.end(),
      [&](bucket_value const &item){
        return item.first == key;});
    }

  public:
    Value value_for(Key const &key, Value const &default_value = Value()) const{
      std::shared_lock<std::shared_mutex> lock(mutex_);
      bucket_iterator const found_entry = find_entry_for(key);
      return (found_entry == data_.end())?
        default_value : found_entry->second;
    }

    void add_or_update_mapping(Key const &key, Value const &value){
      std::unique_lock<std::shared_mutex> lock(mutex_);
      bucket_iterator const found_entry = find_entry_for(key);
      if(found_entry == data_.end()){
        data_.push_back(bucket_value(key,value));
      }else{
        found_entry->second = value;
      }
    }
  };

  std::vector<boost::scoped_ptr<bucket_type>> buckets_;
  Hash hasher_;

  bucket_type &get_bucket(Key const &key) const{
    std::size_t const bucket_index = hasher_(key) % buckets_.size();
    return *buckets_[bucket_index];
  }

public:
  typedef Key key_type;
  typedef Value mapped_type;

  typedef Hash hash_type;
  threadsafe_lookup_table(unsigned num_bukets = 19,
                          Hash const hasher = Hash())
    : buckets_(num_bukets),
      hasher_(hasher_)
  {
    for(unsigned i = 0; i < num_bukets; ++ i){
      buckets_[i].reset(new bucket_type);
    }
  }
  
  threadsafe_lookup_table(const threadsafe_lookup_table &other) = delete;

  threadsafe_lookup_table & operator =(const threadsafe_lookup_table &rhs) = delete;

  mapped_type value_for(key_type const &key, 
                        mapped_type const &default_value) const
  {
    return get_bucket(key).value_for(key, default_value);
  }

  void add_or_update_mapping(key_type const &key, 
                             mapped_type const &value)
  {
    get_bucket(key).add_or_update_mapping(key, value);
  }

  void remove_mapping(key_type const &key){
    get_bucket(key).remove_mapping(key);
  }
};
// namespace jiangbo

template <typename Key, typename Value>
class threadsafe_unordered_map{
public:
  typedef typename std::unordered_map<Key, Value>::iterator iterator;
  Value &operator[](const Key &key){
    std::unique_lock<std::shared_mutex> lock(mutex_);
    return map_[key];
  }

  Value &value_for(const Key &key){
    std::shared_lock<std::shared_mutex> lock(mutex_);
    iterator find_entry = map_.find(key);
    if(find_entry == map_.end()){
      return Value();
    }
    return find_entry->second;
  }
private:
  std::unordered_map<Key, Value> map_;
  mutable std::shared_mutex mutex_;
};

using namespace jiangbo;
threadsafe_lookup_table<int, int> lookup_table;
//std::unordered_map<int, int> map_;

void work1(){
  for(int i = 0; i < 1e8; ++ i){
    lookup_table.add_or_update_mapping(i, i);
  }
}
void work2(){
  for(int i = 0; i < 1e8; i ++){
    lookup_table.value_for(i, i);
  }
}



int main(){
  jiangbo::Timestamp begin(jiangbo::Timestamp::now());
  std::thread thread1(work1);
  std::thread thread2(work2);
  thread1.join();
  thread2.join();
  jiangbo::Timestamp end(jiangbo::Timestamp::now());
  std::cout << "cost " << jiangbo::timeDiffence(begin, end) << " s.";
  return 0;
}