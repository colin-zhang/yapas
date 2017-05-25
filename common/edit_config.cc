#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <iostream>
#include <string>

static std::string& LTrim(std::string& s)
{
     return s.erase(0, s.find_first_not_of(" \t\n\r"));
}

static std::string& RTrim(std::string& s)
{
     return s.erase(s.find_last_not_of(" \t\n\r") + 1);
}

static std::string& Trim(std::string& s)
{
     return RTrim(LTrim(s));
}

static int replaceValueAfterEqualSign(std::string& str, std::string& value)
{
     std::string::size_type n;
     n = str.find('=');
     if (std::string::npos == str.find('=')) {
         value = "";
         return -1;
     }
     str.replace(str.begin() + n + 1 , str.end(), value);
     return 0;
}

static int getKeyValueByEqualSign(std::string& str, std::string& key, 
std::string& value)
{
     std::string::size_type n;
     n = str.find('=');
     if (std::string::npos == str.find('=')) {
         key = "";
         value = "";
         return -1;
     }

     key = str.substr(0, n);
     value = str.substr(n + 1);

     Trim(key);
     Trim(value);

     return 0;
}

static int isFileExist(const char* path)
{
     if (access(path, F_OK) < 0) {
         return 0;
     }
     return 1;
}

int configAppend(std::string fileName, std::string keyValue)
{
     FILE* fpOrigin = fopen(fileName.c_str(), "r+");
     if (fpOrigin == NULL) {
         fprintf(stderr, "can not open[r], %s\n", fileName.c_str());
         return -1;
     }

     std::string key, value;
     std::string lineStr;

     if (getKeyValueByEqualSign(keyValue, key, value) != 0) {
         fprintf(stderr, "%s not right\n", keyValue.c_str());
         return -1;
     }

     int rc = fseek(fpOrigin, -1, SEEK_END);
     char ch = fgetc(fpOrigin);
     if (ch == '\n') {
         lineStr = key + "  =  " + value;
     } else {
         lineStr = "\n" + key + "  =  " + value;
     }

     fwrite(lineStr.c_str(), 1, lineStr.size(), fpOrigin);
     fclose(fpOrigin);
     return 0;
}

int configReplace(std::string fileName, std::string keyValue, int if_del)
{
     std::string fileNameTmp = fileName + ".tmp";
     int timen = 0;

     if (!isFileExist(fileName.c_str())) {
         fprintf(stderr, "%s not exist\n", fileName.c_str());
         return -1;
     }

     FILE* fpOrigin = fopen(fileName.c_str(), "r");
     if (fpOrigin == NULL) {
         fprintf(stderr, "can not open[r], %s\n", fileName.c_str());
         return -1;
     }

     FILE* fpReplace = fopen(fileNameTmp.c_str(), "wb");
     if (fpReplace == NULL) {
         fprintf(stderr, "can not open[r], %s\n", fileNameTmp.c_str());
         return -1;
     }

     std::string key, value;
     std::string lineStr;
     std::string key_infile, value_infile;

     if (getKeyValueByEqualSign(keyValue, key, value) != 0) {
         fprintf(stderr, "%s not right\n", keyValue.c_str());
         return -1;
     }
     value.insert(0, 1, ' ');

     while (!feof(fpOrigin)) {
         char c = fgetc(fpOrigin);
         if (EOF != c) {
             lineStr.append(1, c);
         }
         if ('\n' == c || EOF == c) {
             getKeyValueByEqualSign(lineStr, key_infile, value_infile);
             if (key_infile.compare(key) == 0)
             {
                 int rc = replaceValueAfterEqualSign(lineStr, value);
                 if (rc == 0) {
                     lineStr.append(1, '\n');
                     timen++;
                 }
                 if (if_del) {
                     lineStr.clear();
                 }
             }
             if (lineStr.size() > 0) {
                 fwrite(lineStr.c_str(), 1, lineStr.size(), fpReplace);
             }
             lineStr.clear();
         }
     }

     fclose(fpOrigin);
     fclose(fpReplace);
     if (rename(fileNameTmp.c_str(), fileName.c_str()) < 0 ) {
         fprintf(stderr, "file to rename , %s", strerror(errno));
         return -1;
     }
     return timen;
}

static void help()
{
     printf( "config_edit v1.0.0 \n"
             "\tuse ./config_edit \"config file\" \"key value pair\" \n"
             "\t./config_edit test.cfg KEY1=hello\n"
         );
}

int main(int argc, char const* argv[])
{
     int app = 0;
     int del = 0;

     if (argc != 3 && argc != 4) {
         help();
         exit(-1);
     }
     if (argc == 4) {
         if (strncmp(argv[3], "app", 3) == 0) {
             app = 1;
         } else if (strncmp(argv[3], "del", 3) == 0) {
             del = 1;
         }
     }
     std::string fileName(argv[1]);
     std::string keyValue(argv[2]);
     int n = configReplace(fileName, keyValue, del);
     printf("replace %d value\n", n);
     if (n == 0 && app) {
         int r = configAppend(fileName, keyValue);
         printf("append value, r=%d\n", r);
     }
     return n;
}