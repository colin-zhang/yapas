#include <iostream>  
#include <bitset>  

using namespace std;


static int unrte_cores_count(int coreNum, unsigned char *cores, unsigned long mask)
{
    bitset<128> bs(mask);
    int i, j;

    bs.flip();
    for (i = 0, j = 0; i < bs.size(); i++) {
        if (i >= coreNum) break;
        if (bs.test(i)) {
            cores[j] = i;
            j++;
        }
    }
    return j;
}

int main()
{
    bitset<128> bs(0xf0);

    unsigned char a[128] = {0};
    unsigned long mask = 0xf0;
    int i, j;

    cout << bs.size() << endl;

    /*for (i = 0, j = 0; i < bs.size(); i++) {
        if (bs.test(i)) {
            a[j] = i;
            j++;
        }
    }*/

    mask |= 0x01;
    cout << unrte_cores_count(8, a, mask);
    cout << endl;

    for (i = 0 ; i < 128; i++) {
        cout << static_cast<int>(a[i]) << " ";
    }
    cout << endl;

    return 0;
}