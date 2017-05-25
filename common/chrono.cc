#include <iostream>
#include <iomanip>
#include <ctime>
#include <chrono>
 
using namespace std::chrono;

std::time_t next_hours(int n)
{
    return 0;
}


int main()
{
    system_clock::time_point now = system_clock::now();
    std::time_t last = system_clock::to_time_t(now - std::chrono::hours(24));
    std::time_t next= system_clock::to_time_t(now - std::chrono::hours(24));
    std::time_t now_time= system_clock::to_time_t(now);

    struct tm* n_tm;
    n_tm = std::localtime(&now_time);
    std::time_t nd_time = system_clock::to_time_t(now - 
                                                std::chrono::hours(n_tm->tm_hour) - 
                                                std::chrono::minutes(n_tm->tm_min) - 
                                                std::chrono::seconds(n_tm->tm_sec) +
                                                std::chrono::hours(24) 
                                                );
    int n = 3;
    std::time_t nh_time = system_clock::to_time_t(now + 
                                                std::chrono::hours(n) - 
                                                std::chrono::minutes(n_tm->tm_min) - 
                                                std::chrono::seconds(n_tm->tm_sec)
                                                );
    int64_t nt = now_time;
    std::cout << nt << std::endl; 

    std::cout << "One day ago, the time was "<< std::put_time(std::localtime(&now_time), "%F %T") << '\n';
    std::cout << "One day ago, the time was "<< std::put_time(std::localtime(&last), "%F %T") << '\n';
    std::cout << "Next day,    the time was "<< std::put_time(std::localtime(&next), "%F %T") << '\n';
    std::cout << "Next day,    the time was "<< std::put_time(std::localtime(&nd_time), "%F %T") << '\n';
    std::cout << "Next day,    the time was "<< std::put_time(std::localtime(&nh_time), "%F %T") << '\n';
}
