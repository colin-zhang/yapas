#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <string>
#include <vector>

using namespace std;

class CmdLine
{
    public:
        CmdLine();
        ~CmdLine();
        void init();
        std::string readLine();
		void backSpace(int n);
		void clearLine();
    private:
        struct termios tty_atrr;
        std::vector <std::string> history; 
        vector<string>::iterator curr;
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

void CmdLine::clearLine()
{
    backSpace(cmd.size());
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

void CmdLine::backSpace(int n) 
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

string CmdLine::readLine()
{
    string res("");
    int len = read(in_fd, &c, 1); 
/*
    switch (c)
    {
        case 127:
            break;
        
    }
*/
    if (c == '\n') {
        res = string(cmd);
        history.push_back(cmd);
        cmd.clear();
        write(out_fd, &c, 1);
    } else if (c == 127) {
        if (!cmd.empty()) {
            cmd.erase(cmd.end() - 1);
        }
		backSpace(1);
    } else if (c == 0x1B) {
       char b[2];
       int len = read(in_fd, b, sizeof(b));
       if (b[0] == 0x5B && b[1] == 0x41) {
            printf("up\n");
            fflush(stdout);
       } else  if (b[0] == 0x5B && b[1] == 0x42) {
            printf("down\n");
            fflush(stdout);
       } else {
            printf("\n%02x, %02x", b[0], b[1]);
            fflush(stdout);
       }
    } else if (isprint(c)) {
        write(out_fd, &c, 1);
        cmd.push_back(c);
    } else {
        //printf("\n%02x", c);
        fflush(stdout);
    }
    fsync(out_fd);
    return res;
}

int main()
{
    int a;
    a = isatty(0);
    std::cout << a << std::endl;
    CmdLine cmd;
    while (1) {
        string c;
        c = cmd.readLine();
        if (c.size() > 0) {
            //std::cout << c << std::endl;
            printf("%s \n", c.c_str());
            fflush(stdout);
        }
    }

}

