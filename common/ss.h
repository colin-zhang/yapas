#pragma once
#include <iostream>
#include <stdint.h>
#include <pthread.h>
#include <sys/time.h>

#include <stdio.h>


class CSampele
{
public:
    CSampele() {
        count = 0;
    }
    CSampele(int s) {
        gettimeofday(&tv, NULL);
        count = s;
    }
    uint64_t getTimeMs(void) {
        return static_cast<uint64_t>(tv.tv_sec) * 1000 + tv.tv_usec / 1000;
    }
    void setCount(uint64_t count) {
        gettimeofday(&tv, NULL);
        this->count = count;
    }
    uint64_t getCount(void) {
        return count;
    }
/*
    CSampele &operator=(const CSampele& s1) {
        this->count = s1.count;
        this->tv.tv_sec = s1.tv.tv_sec;
        this->tv.tv_usec = s1.tv.tv_usec;
        return *this;
    }
 */
private:
    struct timeval tv;
    uint64_t count;
};

template <typename T>
class StatisticsQueue
{
public:
    StatisticsQueue(int qmax) : _max(qmax), _current(0), _size(0) {
        v = new T[qmax];
        pthread_mutex_init(&_mu, NULL);
    }
    StatisticsQueue() : _max(60), _current(0), _size(0) {
        v = new T[60];
        pthread_mutex_init(&_mu, NULL);
    }
    ~StatisticsQueue() {
        pthread_mutex_destroy(&_mu);
        delete[] v;
    }

    bool isFull(void) {
        if (_size == _max) {
            return true;
        } else {
            return false;
        }
    }

    bool isEmpty(void) {
        if (_size == 0) {
            return true;
        } else {
            return false;
        }
    }

    int size(void) {
        return _size;
    }

    int max(void) {
        return _max;
    }

    int insert(T t) {
        pthread_mutex_lock(&_mu);
        if (!isFull()) {
            _size++;
        }

        if (_current + 1 == _max) {
            _current = 0;
        } else {
            _current++;
        }
        v[_current] = t;
        pthread_mutex_unlock(&_mu);
        return 0;
    }

    T *get(void) {
        if (!isEmpty()) {
            return &v[_current];
        } else {
            return NULL;
        }
    }

    T *get(int offset) {
        int where;
        if (offset < _size) {
            if (_current < offset) {
                where = _max + _current - offset;
            } else {
                where = _current - offset;
            }
            return &v[where];
        } else {
            return NULL;
        }
    }

private:
    T *v;
    int _current;
    int _size;
    int _max;
    pthread_mutex_t _mu;
};

class CSampleStatistics
{
public:
    CSampleStatistics(int len) : _len(len) {
        sampleQ = new StatisticsQueue<CSampele>(len);
    }

    CSampleStatistics() : _len(60) {
        sampleQ = new StatisticsQueue<CSampele>(60);
    }

    ~CSampleStatistics() {
        delete sampleQ;
    }

    int gatherStatistics(int LL) {
        CSampele *scr = sampleQ->get();
        CSampele *sof = sampleQ->get(LL);
        if (NULL == scr || NULL == sof) {
            return -1;
        }

        uint64_t d_tms = scr->getTimeMs() - sof->getTimeMs();
        uint64_t d_cnt = scr->getCount() - sof->getCount();

        //printf("scr->getTimeMs() = %ld , sof->getTimeMs() = %ld \n", scr->getTimeMs(), sof->getTimeMs());
        //printf("d_tms = %ld , c_cnt = %ld \n", d_tms, d_cnt);

        double rate = static_cast<double>(d_cnt) / d_tms;
        return static_cast<int>(rate * 1000);

    }

    void insert(CSampele s) {
        sampleQ->insert(s);
    }

private:
    int _len;
    StatisticsQueue<CSampele> *sampleQ;
};
