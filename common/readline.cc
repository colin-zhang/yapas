#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <fcntl.h>
#include <string>
#include <vector>

using namespace std;

class CmdLine
{
public:
    CmdLine();
    CmdLine(const char *prompt_cstr);
    ~CmdLine();
    void init();
    string readLine();
    void backSpace(int n);
    void clearLine();
    void writeLine(string &buf);
private:
    struct termios old_tty_atrr;
    vector <string> history; 
    vector<string>::iterator curr;
    string prompt;
    string cmd;
    char c;
    int in_fd;
    int out_fd;
};

CmdLine::CmdLine()
{
    in_fd = STDIN_FILENO;
    out_fd = STDOUT_FILENO;
    prompt = string("> ");
    init();
}

CmdLine::CmdLine(const char *prompt_cstr)
{
    in_fd = STDIN_FILENO;
    out_fd = STDOUT_FILENO;
    prompt = string(prompt_cstr);
    init();
    //int flags = fcntl(in_fd, F_GETFL, 0);  
    //fcntl(in_fd, F_SETFL, flags | O_NONBLOCK);

}

CmdLine::~CmdLine()
{
    tcsetattr(in_fd, TCSANOW, &old_tty_atrr);
}

void CmdLine::clearLine()
{
    backSpace(cmd.size());
}

void CmdLine::writeLine(string &buf)
{
    write(out_fd, buf.c_str(), buf.size());
    //fsync(out_fd);
}

void CmdLine::init()
{
    struct termios tty_attr;
    tcgetattr(in_fd, &tty_attr);
    old_tty_atrr = tty_attr;
    tty_attr.c_lflag &= (~(ICANON|ECHO));
    tty_attr.c_cc[VTIME] = 0;
    tty_attr.c_cc[VMIN] = 1;
    tcsetattr(in_fd, TCSANOW, &tty_attr);
    writeLine(prompt);
}

void CmdLine::backSpace(int n) 
{
    char *s = new char[n];
    char *b = new char[n];
    memset(s, ' ', n);
    memset(b, '\b', n);
    write(out_fd, b, n);
    write(out_fd, s, n);
    write(out_fd, b, n);
    delete []s;
    delete []b;
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
        write(out_fd, &c, 1);
        if (cmd.empty()) {
            writeLine(prompt);
        } else {
            history.push_back(cmd);
            curr = history.end() - 1;
        }
        res = string(cmd);
        cmd.clear();
    } else if (c == 127) {
        if (!cmd.empty()) {
            cmd.erase(cmd.end() - 1);
            backSpace(1);
        }
    } else if (c == 0x1B) {
       char b[2];
       int len = read(in_fd, b, sizeof(b));
       if (b[0] == 0x5B && b[1] == 0x41 && history.size() > 0) {
            //up arrow
            clearLine();
            writeLine(*curr);
            cmd = string(*curr);
            if (curr == history.begin()) {
            } else {
                curr--;
            }
       } else  if (b[0] == 0x5B && b[1] == 0x42 && history.size() > 0) {
            //down arrow
            if (curr == history.end() - 1) {
            } else {
                curr++;
                clearLine();
                writeLine(*curr);
                cmd = string(*curr);
            }
       } else {
            //printf("\n%02x, %02x", b[0], b[1]);
            //fflush(stdout);
       }
    } else if (isprint(c)) {
        write(out_fd, &c, 1);
        cmd.push_back(c);
    } else {
        //printf("\n%02x", c);
        //fflush(stdout);
    }
    //fsync(out_fd);
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
            if (c == "q") {
                break;
            }
        }
    }

}

