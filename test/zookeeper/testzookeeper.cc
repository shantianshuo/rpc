#include "zookeeperutil.h"
#include "mprpcapplication.h"
#include <semaphore.h>
#include <iostream>
int main()
{
    ZkClient zkClient;
    zkClient.Start();
    return 0;
}