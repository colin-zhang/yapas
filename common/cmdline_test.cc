#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <string>
#include <vector>
#include <set>
#include "cmdline.h"

using namespace std;

int main()
{
    int a;
    a = isatty(0);
    std::cout << a << std::endl;
    CmdLine cmd;
    cmd.addHints("help");
    cmd.addHints("ls");
    cmd.addHints("pwd");
    cmd.addHints("env");
    while (1) {
        string c;
        c = cmd.readLine();
        if (c.size() > 0) {
            //std::cout << c << std::endl;
            printf("%s \n", c.c_str());
            printf("> ");
            fflush(stdout);
            if (c == "q") {
                break;
            }
        }
    }

}

