#include <iostream>
#include "ftd2xx.h"

int main()
{
    unsigned int num_devs = 0;
    FT_CreateDeviceInfoList(&num_devs);
    std::cout << "Everything is working!\n";
    return 0;
}
