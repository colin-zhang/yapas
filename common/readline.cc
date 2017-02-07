#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <string>
#include <vector>

class CmdLine
{
    public:
        CmdLine();
        ~CmdLine();
        void init();
        void readline();
		void backspace(int n);
    private:
        struct termios tty_atrr;
        std::vector <std::string> history; 
        std::string cmd;
        char c;
        int in_fd;
        int out_fd;
};

CmdLine::CmdLine()
{
    in_fd = STDIN_FILENO;
    out_fd = STDOUT_FILENO;
    init();
}

CmdLine::~CmdLine()
{

}

void CmdLine::init()
{
    struct termios tty_attr;
    tcgetattr(in_fd, &tty_attr);
    tty_attr.c_lflag &= (~(ICANON|ECHO));
    tty_attr.c_cc[VTIME] = 0;
    tty_attr.c_cc[VMIN] = 1;
    tcsetattr(in_fd, TCSANOW, &tty_attr);
}

void CmdLine::backspace(int n) 
{
	int i;
	for (i = 0; i < n; i++) {
		char b = '\b';
        write(out_fd, &b, 1);
        b = ' ';
        write(out_fd, &b, 1);
		b = '\b';
        write(out_fd, &b, 1);
	}	
}

void CmdLine::readline()
{
    int len = read(in_fd, &c, 1); 
/*
    switch (c)
    {
        case 127:
            break;
        
    }
*/
    if (c == 127) {
		backspace(1);
    } else {
        //write(out_fd, &c, 1);
        printf("\n%02x", c);
        fflush(stdout);
    }
    
    fsync(out_fd);
}

int main()
{
    int a;
    a = isatty(0);
    std::cout << a << std::endl;
    CmdLine cmd;
    while (1) {
        cmd.readline();
    }

}

