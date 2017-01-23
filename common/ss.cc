#include "ss.h"
#include <unistd.h>
#include <string>

struct my_kv
{
    int age;
    char name[16];
};

struct myST {
    struct timeval tv[2];
    struct my_kv kv[2];
};

class MainST {
public:
    StatisticsQueue<myST> sq;
    MainST() : sq(10) {
    }
};

#if 0

int main()
{
    StatisticsQueue<int> *ss = new StatisticsQueue<int>(3);
    StatisticsQueue<int> sq(10);

    //sq = *ss;

    std::cout << sq.isFull() << std::endl;
    std::cout << sq.size() << std::endl;
    std::cout << sq.max() << std::endl;

    std::cout << sq.get() << std::endl;

    sq.insert(1);
    sq.insert(2);
    sq.insert(3);
    sq.insert(4);

    std::cout << *sq.get(0) << std::endl;
    std::cout << *sq.get(1) << std::endl;
    std::cout << *sq.get(2) << std::endl;

    return 0;
}

#else

int main()
{
#if 0
    CSampleStatistics ss(10);
    CSampele s(2000);

    ss.insert(s);
    sleep(1);
    s.setCount(10100);
    ss.insert(s);

    std::cout << s.getTimeMs() << std::endl;
    std::cout << ss.gatherStatistics(1) << std::endl;
#else
    MainST mst;
    struct myST d;

    gettimeofday(&d.tv[0], NULL);
    gettimeofday(&d.tv[1], NULL);

    d.kv[0].age = 20;
    sprintf(d.kv[0].name, "%s", "colin");


    d.kv[1].age = 30;
    sprintf(d.kv[1].name, "%s", "tim");


    mst.sq.insert(d);
    memset(&d, 0, sizeof(d));
    struct myST *dd;

    dd = mst.sq.get();

    printf("%d , %s\n", dd->kv[0].age, dd->kv[0].name);
    printf("%d , %s\n", dd->kv[1].age, dd->kv[1].name);

    char my[16] = "hello";

    if (std::string(my) == "hello") {
        printf("%s\n", my);
    } else {
        printf("%s\n", "hehe");
    }


#endif
}

#endif
