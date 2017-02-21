#include <iostream>
#include <pthread.h>
#include <cstdio>
#include <map>
#include <string>

#define N 10000

template <typename _K, typename _V>
class Counter
{
 public:
    Counter() : counter_(0) {
        pthread_rwlock_init(&lock_, NULL);
    }

    ~Counter() {
        pthread_rwlock_destroy(&lock_);
    }

    void rlock() {
        pthread_rwlock_rdlock(&lock_);
    }

    void wlock() {
        pthread_rwlock_wrlock(&lock_);
    }
    
    void unlock() {
        pthread_rwlock_unlock(&lock_);
    }
    
    //std::map<_K, _V>::size_type 
    long size() {
        return map_.size();
    }

/* 
    std::map<_K, _V>::const_iterator begin() {
        return map_.begin();
    }
    std::map<_K, _V>::iterator end() {
        return map_.end();
    }
*/

    std::map<_K, _V>& get() {
        return map_;
    }
    
    void add(int n) {
        for (int i = 0; i < n; i++) {
            pthread_rwlock_wrlock(&lock_);
            counter_++;
            pthread_rwlock_unlock(&lock_);
        }
    }

    void print() {
        printf("counter is %d, map.size = %ld \n", counter_, map_.size());
    }

    void clear() {
        pthread_rwlock_wrlock(&lock_);
        map_.clear();
        pthread_rwlock_unlock(&lock_);
    }

    void operator[] (_K key) {
    }

    void insert(_K& key, _V& v) {
        pthread_rwlock_wrlock(&lock_);
        map_.insert(std::pair<_K, _V>(key, v));
        pthread_rwlock_unlock(&lock_);
    }
 public:
    std::map<_K, _V> map_;
 private:
     int counter_;
     pthread_rwlock_t lock_;
};

static int a;
static Counter<std::string, int> counter;

void* test(void* ptr)
{
    int i;
    char buf[128] = {0};
    for (i = 0; i < 10000; i++) {
        a = a + 1;
        snprintf(buf, sizeof buf, "ID_%d", i);
        std::string a(buf);
        counter.insert(a, i);
        if ( i == 5000) {
            counter.clear();
        }
    }
    counter.add(10000);
    //printf("a is %d \n", a);
    return NULL;
}
int main()
{
    int i;
    pthread_t pthread_id[N]; 
    for (i = 0; i < 10; i++) {
        pthread_create(&pthread_id[i], NULL, test, NULL);
    }

    for (i = 0; i < 10; i++) {
        pthread_join(pthread_id[i], NULL);
    }
    printf("a is %d \n", a);
    counter.print();

    std::map<std::string, int>::iterator it = counter.map_.begin();
    for ( ; it != counter.map_.end(); it++) {
        std::cout << it->first << "   " << it->second << std::endl;
    }

    return 0;
}
