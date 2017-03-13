#ifndef _CQMS_CMD_LINE_H
#define _CQMS_CMD_LINE_H
#include <unistd.h>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <termios.h>
#include <string.h>
#include <string>
#include <vector>
#include <set>

/*class HintCompare 
{
public:
    bool operator() (const string &a, const string &b) const {
        //int n = b.size() > a.size() ? a.size() : b.size();
        int n = a.size();
        printf("\n%s\n", a.c_str());
        if (strncmp(a.c_str(), b.c_str(), n) < 0) {
            return true;
        } else {
            return false;
        }
    }
};*/

class CmdLine
{
public:
    CmdLine();
    CmdLine(const char *prompt_cstr);
    ~CmdLine();
    void init();
    std::string readLine();
    void backSpace(int n);
    void clearLine();
    void writeLine(std::string& buf);
    void addHints(std::string hint);
    int  compareHints(std::string* possible, std::string* append_str);
private:
    struct termios old_tty_atrr;
    std::vector <std::string> history; 
    std::vector<std::string>::iterator curr;
    //set<string, HintCompare> hints;
    std::vector<std::string> hints;
    std::string prompt;
    std::string cmd;
    std::string cmd_buffer;
    char c;
    int in_fd;
    int out_fd;
    char last_key;
};

#endif
