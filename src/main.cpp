//
// Created by uuu on 2022/7/30.
//

#include <iostream>
#include <unistd.h>
#include "srs_client.h"


int main(int argc, char *argv[])
{
    SrsClient sc;
    sc.Start();

    while (true) {
        pause();
    }

    return 0;
}