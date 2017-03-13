#include "cmdline.h"

using namespace std;

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
    cmd_buffer = "";
    init();
    //int flags = fcntl(in_fd, F_GETFL, 0);  
    //fcntl(in_fd, F_SETFL, flags | O_NONBLOCK);
}

CmdLine::~CmdLine()
{
    tcsetattr(in_fd, TCSANOW, &old_tty_atrr);
}

void CmdLine::addHints(string hint)
{
    std::vector<string>::iterator it;
    for (it = hints.begin(); it != hints.end(); it++) {
        if (*it == hint) {
            break;
        }
    }
    if (it == hints.end()) {
        hints.push_back(hint);
    }
    //hints.insert(hint);
}

void CmdLine::clearLine()
{
    backSpace(cmd.size());
}

void CmdLine::writeLine(string& buf)
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
    history.push_back("help");
    curr = history.end();
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

int  CmdLine::compareHints(std::string* possible, std::string* append_str)
{   
    int most_len = 65535;
    std::vector<string> common;
    std::vector<string>::iterator it = hints.begin();
    possible->clear();
    for ( ; it != hints.end(); it++) {
        if (it->length() > cmd.length()) {
            if (0 == it->compare(0, cmd.length(), cmd)) {
                common.push_back(*it);
                possible->append(*it);
                possible->append(" ");
                if (it->length() < most_len) {
                    most_len = it->length();
                }
            }
        }
    }
    if (common.size() < 2) {
        return common.size();
    }

    string common_str("");
    char x = 0, xx = 0;
    for (int i = cmd.length(); i < most_len; i++) {
        it = common.begin();
        x = (*it)[i];
        for ( ; it != common.end(); it++) {
            xx = (*it)[i];
            if (xx != x) {
                x = 0;
                break;
            }
        }
        if (x) {
            common_str.append(1, x);
        } else {
            break;
        }
    }
    append_str->append(common_str);
    return common.size();
}

string CmdLine::readLine()
{
    string res("");
    int len = read(in_fd, &c, 1); 

    if (c == '\n') {
        write(out_fd, &c, 1);
        cmd.erase(0, cmd.find_first_not_of(" "));
        if (cmd.empty()) {
            writeLine(prompt);
        } else if (*(history.end()-1) != cmd){
            history.push_back(cmd);
            curr = history.end();
        } else {
            curr = history.end();
        }
        res = string(cmd);
        cmd.clear();
        cmd_buffer.clear();
    } else if (c == '\t') {
        int count = 0;
        string possible_cmd, append_str;
        possible_cmd.clear();
        append_str.clear();
        count = compareHints(&possible_cmd, &append_str);
        if (count > 1 && last_key == c ) {
            possible_cmd.insert(0, "\n");
            possible_cmd.append("\n");
            writeLine(possible_cmd);
            writeLine(prompt);
            cmd.append(append_str);
            writeLine(cmd);
            //cmd.clear();
        } else if (1 == count) {
            clearLine();
            writeLine(possible_cmd);
            cmd = possible_cmd;
            last_key = 0;
        } else {
            //last_key = 0;
        }
        //set<string, HintCompare>::iterator it = hints.find(cmd);
/*        if (it != hints.end()) {
            //if (it->length() > cmd.length()) {
                clearLine();
                writeLine(*it);
                cmd = *it;
            //}
        }*/
    } else if (c == 127 || c == 8) {
        if (!cmd.empty()) {
            cmd.erase(cmd.end() - 1);
            backSpace(1);
        }
    } else if (c == 0x1B) {
       char b[2];
       int len = read(in_fd, b, sizeof(b));
       if (b[0] == 0x5B && b[1] == 0x41 && history.size() > 0) {
            //up arrow
            if (curr == history.begin()) {
            } else {
                curr--;
                if (*curr == cmd && curr != history.begin()) {
                   curr--; 
                }
            }
            clearLine();
            writeLine(*curr);
            cmd = string(*curr);
       } else  if (b[0] == 0x5B && b[1] == 0x42 && history.size() > 0) {
            //down arrow
            if (curr == history.end()) {
            } else if (curr == history.end() - 1) {
                if (cmd_buffer.size() != 0) {
                    clearLine();
                    writeLine(cmd_buffer);
                    cmd = cmd_buffer;
                }
                curr++;
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
        cmd_buffer = cmd;
    } else {
        //printf("\n%02x", c);
        //fflush(stdout);
    }
    last_key = c;
    //fsync(out_fd);
    return res;
}
