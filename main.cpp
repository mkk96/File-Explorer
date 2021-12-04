#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <string.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <vector>
#include <algorithm>
#include <string.h>
#include <termios.h>
#include <stack>
#include <sys/ioctl.h>
using namespace std;
#define BLOCK_SIZE 4096
#define cursorupward(x) printf("\033[%dA", (x))
#define cursordownward(x) printf("\033[%dB", (x))
#define KEY_ESCAPE  0x001b
#define KEY_ENTER   0x000a
#define KEY_UP      0x0105
#define KEY_DOWN    0x0106
#define KEY_LEFT    0x0107
#define KEY_RIGHT   0x0108
#define BACKSPACE      127

struct termios newT, oldT;
int kbhit(void);
int kbesc(void);
int getch(void);
int kbget(void);
struct fileAndFolder{
    char *display;
    char *path;
};
void trimPath(char *source, char *destination){
    size_t source_length = strlen(source);
    size_t destination_length = strlen(destination);
    int indexs=0;
    int indexd=destination_length;
    for(indexs=source_length-1;indexs>=0;indexs--){
        if(*(source+indexs)=='/')
            break;
    }
    for(;indexs<source_length;indexs++,indexd++)
        *(destination+indexd)=*(source+indexs);
    *(destination+indexd)='\0';
}
int makeDir(char *path){
    struct stat status_of_directory = {0};
    if (stat(path, &status_of_directory) == -1) {
        mkdir(path, 0700);
        return 1;
    }   
    return 0;
}
void makeSDir(char *pathCopy, char *pathNew){
    trimPath(pathCopy, pathNew);
    makeDir(pathNew);
}

int copyBlock(char *path1, char *path2){
    char block[BLOCK_SIZE];
    int input, output;
    int read_data;
    trimPath(path1,path2);
    input = open(path1,O_RDONLY);
    output = open(path2, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
    struct stat status_of_file;
    stat(path1, &status_of_file);
    chmod(path2, status_of_file.st_mode);
    while((read_data=read(input, block, sizeof(block)))>0)
        write(output, block, read_data);
    return 0;
}

int copyFIDir(char *pathCopy, char *pathNew){
    DIR *pointerPathCopy;
    DIR *pointerPathNew;
    size_t lengthPathCopy = strlen(pathCopy);
    size_t lengthPathNew = strlen(pathNew);
    int return_status = -1;
    if((pointerPathCopy = opendir(pathCopy)) == NULL || (pointerPathNew = opendir(pathNew))==NULL)
        return 0;
    chdir(pathCopy);
    chdir(pathNew);
    if(pointerPathCopy && pointerPathNew){
        struct dirent *pointerPathFold;
        return_status = 0;
        while(!return_status && (pointerPathFold=readdir(pointerPathCopy))){
            int statFileFold = -1;
            char buffer1[1000];
            char buffer2[1000];
            size_t pointerPathOFold;
            size_t lengthPointerPathOFold;
            if(!strcmp(".",pointerPathFold->d_name) || !strcmp("..", pointerPathFold->d_name))
                continue;
            pointerPathOFold = lengthPathCopy + strlen(pointerPathFold->d_name) + 2;
            lengthPointerPathOFold = lengthPathNew + strlen(pointerPathFold->d_name) + 2;
            if(buffer1 && buffer2){
                struct stat statbuffer;
                snprintf(buffer1, pointerPathOFold, "%s/%s", pathCopy, pointerPathFold->d_name);
                snprintf(buffer2,lengthPointerPathOFold,"%s/%s", pathNew, pointerPathFold->d_name);
                if(!stat(buffer1, &statbuffer)){
                    if(S_ISDIR(statbuffer.st_mode)){
                        if(makeDir(buffer2)){
                            struct stat status_of_file_or_folder;
                            makeSDir(buffer1, buffer2);
  			     statFileFold=copyFIDir(buffer1, buffer2);
                            stat(buffer1, &status_of_file_or_folder);
                            chmod(buffer2, status_of_file_or_folder.st_mode);
                        }
                        else
                            return 0;
                    }
                    else{
                        buffer2[lengthPathNew]='\0';
                        statFileFold = copyBlock(buffer1,buffer2);
                    }
                }
            }
            return_status = statFileFold;
        }
        chdir("..");
        closedir(pointerPathCopy);
        return return_status;
    }
}
int copyDir(char *pathCopy, char *pathNew){
    makeSDir(pathCopy, pathNew);
    return copyFIDir(pathCopy, pathNew);
}
int createDir(char *dirName, char *path_for_directory){
    struct stat status = {0};
    strcat(path_for_directory, "/");
    strcat(path_for_directory,dirName);
    if (stat(path_for_directory, &status) == -1) {
        mkdir(path_for_directory, 0700);
        return 1;
    }   
    return 0;
}
int createFile(char *filename, char *pathFile){
    strcat(pathFile, "/");
    strcat(pathFile,filename);
    return open(pathFile, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
}

int deleteDir(char *pathDir){
    DIR *pointPath;
    size_t lengthPath = strlen(pathDir);
    int return_status = -1;
    if((pointPath = opendir(pathDir)) == NULL){
        fprintf(stdout, "cannot open directory: %s\n", pathDir);
        return 0;
    }
    chdir(pathDir);
    if(pointPath){
        struct dirent *pointFile;
        return_status=0;
        while(!return_status && (pointFile=readdir(pointPath))){
            int statfold = -1;
            char *buffer;
            size_t len;
            if(!strcmp(pointFile->d_name,".") || !strcmp(pointFile->d_name,".."))
                continue;
            len = lengthPath + strlen(pointFile->d_name)+2;
            buffer = (char *)malloc(len);

            if(buffer){
                struct stat statbuffer;
                snprintf(buffer, len, "%s/%s", pathDir, pointFile->d_name);
                if(!stat(buffer, &statbuffer)){
                    if(S_ISDIR(statbuffer.st_mode))
                        statfold = deleteDir(buffer);
                    else
                        statfold = unlink(buffer);
                }
                free(buffer);
            }
            return_status = statfold;
        }
        chdir("..");
        closedir(pointPath);
        if(!return_status)
            return_status = rmdir(pathDir);
        return return_status;
    }
}
//Complete Till Here
int  deleteFile(char *pathFile){
    return remove(pathFile);  
}
void searchFoDir(char *pathCurr, char *fileFoldName, char *rootName){
    DIR *pointNCurrDir;
    struct dirent *entry;
    struct stat statbuf;

    if((pointNCurrDir = opendir(pathCurr)) == NULL){
        fprintf(stderr, "cannot open directory: %s\n", pathCurr);
        return;
    }
    chdir(pathCurr);
    while((entry=readdir(pointNCurrDir))!=NULL){
        char *buffer;
        size_t pathLen = strlen(pathCurr) + strlen(entry->d_name) + 2;
        buffer = (char *)malloc(pathLen);
        snprintf(buffer, pathLen, "%s/%s", pathCurr, entry->d_name);
        if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
                continue;
        if(!strcmp(fileFoldName, entry->d_name)){
            int i = strlen(rootName);
            printf(".");
            for(;i<strlen(buffer);i++)
                printf("%c",buffer[i]);
            printf("\n\n");
        }
        lstat(entry->d_name, &statbuf);
        if(S_ISDIR(statbuf.st_mode)){
            searchFoDir(buffer, fileFoldName,rootName);
            //printf("%s", buffer);
        }
    }
    chdir("..");
    closedir(pointNCurrDir);
}
int gotoDir(char *pathCurr, char *dirName){
    DIR *pointNCurrDir;
    struct dirent *entry;
    struct stat statbuf;

    if((pointNCurrDir = opendir(pathCurr)) == NULL){
        //fprintf(stderr, "cannot open directory: %s\n", pathCurr);
        return 0;
    }
    chdir(pathCurr);
    while((entry=readdir(pointNCurrDir))!=NULL){
        char *buffer;
        size_t pathLen = strlen(pathCurr) + strlen(entry->d_name) + 2;
        buffer = (char *)malloc(pathLen);
        snprintf(buffer, pathLen, "%s/%s", pathCurr, entry->d_name);
        if(strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
                continue;
        lstat(entry->d_name, &statbuf);
        if(S_ISDIR(statbuf.st_mode)){
            if(!strcmp(dirName, entry->d_name)){
                closedir(pointNCurrDir);
                return 1;
            }
            searchFoDir(buffer, dirName,pathCurr);
            //printf("%s", buffer);
        }
        else{
            if(!strcmp(dirName, entry->d_name)){
                char *s=(char * )malloc(sizeof(buffer) + 100);
                //snprintf(s, sizeof(buffer)+100, "%s %s", "xdg-open", buffer);
                int pid = fork();
                if (pid == 0) {
                    execl("/usr/bin/xdg-open", "xdg-open", buffer, (char *)0);
                    exit(1);
                }
                //system(s);
                closedir(pointNCurrDir);
                return 2;
            }
        }
    }
    chdir("..");
    closedir(pointNCurrDir);
    return 0;
}


bool compName(struct fileAndFolder p1, struct fileAndFolder p2){
    return strcmp(p1.path,p2.path)<0;
}

void fODirPermission(char *path, char *fileFoldName, char *display){
    struct stat statbuf;
    char *buffer1;
    size_t pointerPathOFold;
    pointerPathOFold = strlen(path) + strlen(fileFoldName) + 2;
    buffer1 = (char *)malloc(pointerPathOFold);
    snprintf(buffer1, pointerPathOFold, "%s/%s", path, fileFoldName);
    lstat(buffer1, &statbuf);
    //printf("%d ",statbuf.st_mode);
    if(S_ISDIR(statbuf.st_mode))
        strcat(display," d");
    else
        strcat(display," -");
    if(S_IRUSR&statbuf.st_mode)
         strcat(display,"r");
    else
       strcat(display,"-");
    if(S_IWUSR&statbuf.st_mode)
        strcat(display,"w");
    else
        strcat(display,"-");
    if(S_IXUSR&statbuf.st_mode)
        strcat(display,"x");
    else
        strcat(display,"-");
    if(S_IRGRP&statbuf.st_mode)
        strcat(display,"r");
    else
        strcat(display,"-");
    if(S_IWGRP&statbuf.st_mode)
        strcat(display,"w");
    else
        strcat(display,"-");
    if(S_IXGRP&statbuf.st_mode)
        strcat(display,"x");
    else
        strcat(display,"-");
    if(S_IROTH&statbuf.st_mode)
        strcat(display,"r");
    else
        strcat(display,"-");
    if(S_IWOTH&statbuf.st_mode)
        strcat(display,"w");
    else
        strcat(display,"-");
    if(S_IXOTH&statbuf.st_mode)
        strcat(display,"x ");
    else
        strcat(display,"- ");
    char temp[10];
    strcat(display,getpwuid(statbuf.st_uid)->pw_name);
    strcat(display," ");
    strcat(display,getgrgid(statbuf.st_gid)->gr_name);
    strcat(display," ");
    sprintf(temp, "%8lu ", statbuf.st_size);
    strcat(display, temp);
    time_t t = statbuf.st_mtime;
    struct tm lt;
    localtime_r(&t, &lt);
    char timbuf[80];
    strftime(timbuf, sizeof(timbuf), "%c", &lt);
    strcat(display," ");
    strcat(display, timbuf);
    strcat(display,"\0");
}

vector<struct fileAndFolder> listDirectory(char *path){
    DIR *pointPath;
    struct dirent *entry;
    vector<struct fileAndFolder> lst;
    if((pointPath = opendir(path)) == NULL){
        return lst;
    }
    while((entry=readdir(pointPath))!=NULL){
        struct fileAndFolder res;
        char *dsplay = (char *)malloc(100);
        memset(dsplay,0,sizeof(dsplay));
        fODirPermission(path,entry->d_name, dsplay);
        res.display = dsplay;
        res.path = entry->d_name;
        lst.push_back(res);
    }
    chdir("..");
    closedir(pointPath);
    sort(lst.begin(),lst.end(),compName);
    return lst;
}



int moveDir(char *pathMove, char *pathNew){
    copyDir(pathMove, pathNew);
    return deleteDir(pathMove);
}
int moveFile(char *path1, char *path2){
    copyBlock(path1,path2);
    remove(path1);
    return 0;
}
int reName(char *oldN, char *newN){
    int status;
    status = rename(oldN,newN);
    return status==0? 1:0;
}
int getch(void)
{
    int c = 0;

    tcgetattr(0, &oldT);
    memcpy(&newT, &oldT, sizeof(newT));
    newT.c_lflag &= ~(ICANON | ECHO);
    newT.c_cc[VMIN] = 1;
    newT.c_cc[VTIME] = 0;
    tcsetattr(0, TCSANOW, &newT);
    c = getchar();
    tcsetattr(0, TCSANOW, &oldT);
    return c;
}

int kbhit(void)
{
    int c = 0;

    tcgetattr(0, &oldT);
    memcpy(&newT, &oldT, sizeof(newT));
    newT.c_lflag &= ~(ICANON | ECHO);
    newT.c_cc[VMIN] = 0;
    newT.c_cc[VTIME] = 1;
    tcsetattr(0, TCSANOW, &newT);
    c = getchar();
    tcsetattr(0, TCSANOW, &oldT);
    if (c != -1) ungetc(c, stdin);
    return ((c != -1) ? 1 : 0);
}

int kbesc(void)
{
    int c;
    if (!kbhit()) return KEY_ESCAPE;
    c = getch();
    if (c == '[') {
        switch (getch()) {
            case 'A':
                c = KEY_UP;
                break;
            case 'B':
                c = KEY_DOWN;
                break;
            case 'D':
                c = KEY_LEFT;
                break;
            case 'C':
                c = KEY_RIGHT;
                break;
            default:
                c = 0;
                break;
        }
    } else {
        c = 0;
    }
    if (c == 0) while (kbhit()) getch();
    return c;
}

int kbget(void)
{
    int c;

    c = getch();
    return (c == KEY_ESCAPE) ? kbesc() : c;
}

void gotoxy(int x,int y)
{
    printf("%c[%d;%df",0x1B,x,y);
    fflush(stdout);
}
vector<string> split(string phrase, string delimiter){
    vector<string> list;
    string s = string(phrase);
    size_t pos = 0;
    string token;
    while ((pos = s.find(delimiter)) != string::npos) {
        token = s.substr(0, pos);
        list.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    list.push_back(s);
    return list;
}
void header(int *colNum,string s)
{
    printf("\033[H\033[J");
    printf("\033[3J");
    gotoxy(0,*colNum/2 -5);
    printf("\u001b[0m\u001b[7m ");cout<<s;printf(" \u001b[0m\n");
}
void display(int *start,int *end,vector<struct fileAndFolder> &lst,int *colNum)
{
    for(int i=*start;i<*end && i<lst.size();i++){
        cout<<lst[i].display<<" ";
        if(strlen(lst[i].display)+strlen(lst[i].path)<*colNum)
            cout<<lst[i].path<<"\n";
        else{
            for(int j=0;(strlen(lst[i].display) + j + 1 < *colNum);j++)
                cout<<lst[i].path[j];
            cout<<"\n";
        }
    }
}
void quitprogram()
{
    printf("\033[H\033[J");
    printf("\033[3J");
    gotoxy(0,0);
    exit(0);
}
void ioctlcall(int *rowNum,int *colNum)
{
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    *rowNum = w.ws_row;
    *colNum = w.ws_col;
}
string commandmode(char *root, char *dirRoor, int start, int end){
    int rowNum,colNum;
    ioctlcall(&rowNum,&colNum);
    header(&colNum,"COMMAND MODE");
    vector<struct fileAndFolder> lst;
    lst = listDirectory(root);
    display(&start,&end,lst,&colNum);
    gotoxy(rowNum,0);
    int c;
    char cstrpath[1000];
    char cstrdest[1000];
    while(1){
        ioctlcall(&rowNum,&colNum);
        lst = listDirectory(root);
        header(&colNum,"COMMAND MODE");
        display(&start,&end,lst,&colNum);
        gotoxy(rowNum,0);
        string str;
        while((c=kbget())!='\n' && c!=KEY_ESCAPE){
            if(c == 'q' || c == 'Q')
		    {
		        quitprogram();
		    }
            if(c == BACKSPACE)
                str = str.substr(0,str.size()-1);
            else
                str = str + (char)c;
            header(&colNum,"COMMAND MODE");
            display(&start,&end,lst,&colNum);
            gotoxy(rowNum,0);
            cout<<str;
        }
        if(c==KEY_ESCAPE)
            return root;
        if(c == 'q' || c == 'Q')
        {
                quitprogram();
        }
        vector<string> command;
        if(str[0] != KEY_ESCAPE){
            command = split(str, " ");
            if(command[0] == "copy"){
                string totalPath;
                string dest = command[command.size()-1];
                if(dest[0]=='~'){
                    totalPath = dirRoor + dest.substr(1);
                }
                else{
                    if(dest[0]!='/'){
                        totalPath = root;
                        totalPath.append("/");
                        totalPath = totalPath + dest;
                    }
                    else
                        totalPath = dirRoor + dest;
                }
                for(int i=1;i<command.size()-1;i++){
                    if(command[i].find('.')!=string::npos){
                        string str1 = root;
                        string str2 = totalPath;
                        if(command[i][0]!='/'){
                            str1.append("/");
                            str1 = str1 +  command[i];
                        }
                        else
                            str1 = dirRoor + command[i];
                        strcpy(cstrpath, str1.c_str());
                        strcpy(cstrdest, str2.c_str());
                        copyBlock(cstrpath,cstrdest);
                    }
                    else{
                        string str1 = root;
                        string str2 = totalPath;
                        if(command[i][0]!='/'){
                            str1.append("/");
                            str1 = str1 +  command[i];
                        }
                        else
                            str1 = dirRoor + command[i];
                        strcpy(cstrpath, str1.c_str());
                        strcpy(cstrdest, str2.c_str());
                        copyDir(cstrpath,cstrdest);
                    }
                }
            }
            else if(command[0] == "move"){
                string totalPath;
                string dest = command[command.size()-1];
                if(dest[0]=='~'){
                    totalPath = dirRoor + dest.substr(1);
                }
                else{
                    if(dest[0]!='/'){
                        totalPath = root;
                        totalPath.append("/");
                        totalPath = totalPath + dest;
                    }
                    else
                        totalPath = dirRoor + dest;
                }
                for(int i=1;i<command.size()-1;i++){
                    if(command[i].find('.')!=string::npos){
                        string str1 = root;
                        string str2 = totalPath;
                        if(command[i][0]!='/'){
                            str1.append("/");
                            str1 = str1 +  command[i];
                        }
                        else
                            str1 = dirRoor + command[i];
                        strcpy(cstrpath, str1.c_str());
                        strcpy(cstrdest, str2.c_str());
                        moveFile(cstrpath,cstrdest);
                    }
                    else{
                        string str1 = root;
                        string str2 = totalPath;
                        if(command[i][0]!='/'){
                            str1.append("/");
                            str1 = str1 +  command[i];
                        }
                        else
                            str1 = dirRoor + command[i];
                        strcpy(cstrpath, str1.c_str());
                        strcpy(cstrdest, str2.c_str());
                        moveDir(cstrpath,cstrdest);
                    }
                }
            }
            else if(command[0] == "rename"){
                string str1 = root,str2=root;
                if(command[1][0]!='/'){
                    str1.append("/");
                    str1 = str1 +  command[1];
                }
                else
                    str1 = dirRoor + command[1];
                if(command[2][0]!='/'){
                    str2.append("/");
                    str2 = str2 + command[2];
                }
                else
                    str2 = dirRoor + command[2];
                strcpy(cstrpath, str1.c_str());
                strcpy(cstrdest, str2.c_str());
                reName(cstrpath,cstrdest);
            }
            else if(command[0] == "create_file"){
                string totalPath;
                string dest = command[command.size()-1];
                if(dest[0]=='~')
                    totalPath = root + dest.substr(1);
                else if(dest==".")
                    totalPath = root;
                else if(dest[0]=='/')
                    totalPath = dirRoor + dest;
                else{
                    totalPath = root;
                    totalPath.append("/");
                    totalPath = totalPath + dest;
                }
                for(int i=1;i<command.size()-1;i++){
                        string str1 = command[i];
                        strcpy(cstrpath, str1.c_str());
                        strcpy(cstrdest, totalPath.c_str());
                        createFile(cstrpath, cstrdest);
                }
            }
            else if(command[0] == "create_dir"){
                string totalPath;
                string dest = command[command.size()-1];
                if(dest[0]=='~')
                    totalPath = dirRoor + dest.substr(1);
                else if(dest==".")
                    totalPath = root;
                else if(dest[0]=='/')
                    totalPath = dirRoor + dest;
                else{
                    totalPath = root;
                    totalPath.append("/");
                    totalPath = totalPath + dest;
                }
                for(int i=1;i<command.size()-1;i++){
                        string str1 = command[i];
                        strcpy(cstrpath, str1.c_str());
                        strcpy(cstrdest, totalPath.c_str());
                        createDir(cstrpath, cstrdest);
                }
            }
            else if(command[0] == "delete_file"){
                for(int i=1;i<command.size();i++){
                    string str1 = dirRoor;
                    if(command[i][0]=='~')
                        str1 = str1 + command[i].substr(1);
                    else{
                        if(command[i][0]!='/'){
                            str1 = root;
                            str1.append("/");
                            str1 = str1 + command[i];
                        }
                        else
                            str1 = dirRoor + command[i];
                    }
                    strcpy(cstrpath, str1.c_str());
                    deleteFile(cstrpath);
                }
            }
            else if(command[0] == "delete_dir"){
                for(int i=1;i<command.size();i++){
                    string str1 = dirRoor;
                    if(command[i][0]=='~')
                        str1 = str1 + command[i].substr(1);
                    else{
                        if(command[i][0]!='/'){
                            str1 = root;
                            str1.append("/");
                            str1 = str1 + command[i];
                        }
                        else
                            str1 = dirRoor +  command[i];
                    }
                    strcpy(cstrpath, str1.c_str());
                    deleteDir(cstrpath);
                }
            }
            else if(command[0] == "goto"){
                string str1;
                if(command[1]=="/"){
                    return dirRoor;
                }
                if(command[1][0]=='~'){
                    str1 = dirRoor;
                    str1 = str1 + command[1].substr(1);
                }
                else if(command[1][0]!='/'){
                    str1 = root;
                    str1.append("/");
                    str1 = str1 + command[1];
                }
                else
                    str1 = dirRoor + command[1];
                strcpy(cstrpath, command[1].c_str());
                return str1;
            }
            else if(command[0] == "search"){
                string str1 = root;
                string str2 = command[1];
                strcpy(cstrpath, str1.c_str());
                strcpy(cstrdest, str2.c_str());
                header(&colNum,"NORMAL MODE");
                searchFoDir(cstrpath,cstrdest,cstrpath);
                gotoxy(2,0);
                while(1){
                    c=kbget();
                    if(c == BACKSPACE)
                        return root;
                }
            }
            else if(command[0]=="q" ||command[0]=="Q" ){
                quitprogram();
            }
        }
        else
            break;
    }
    return root;
}
int main(void)
{
    int c;
    char root[500];
    vector<struct fileAndFolder> lst;
    stack<string> prev;
    stack<string> next;
    int row=2;
    int rowNum ,colNum;
    int start,end;
    printf("\033[H\033[J");
    printf("\033[3J");
    getcwd(root,100);
    lst = listDirectory(root);
    ioctlcall(&rowNum,&colNum);
    header(&colNum,"NORMAL MODE");
    start = 0;
    end = rowNum-2;
    display(&start,&end,lst,&colNum);
    prev.push(root);
    gotoxy(2,0);
    char pathN[500];
    strcpy(pathN, root);
     while (c!='q') {
        c = kbget();
        if (c == KEY_ENTER){
            if(row==3 && strcmp(pathN,root)){
                ioctlcall(&rowNum,&colNum);
                header(&colNum,"NORMAL MODE");
                int i=0;
                lst.clear();
                for(i=strlen(pathN)-1;i>=0;i--){
                    if(pathN[i]=='/')
                        break;
                }
                pathN[i]='\0';
                string path = pathN;
                prev.push(path);
                while(!next.empty())
                    next.pop();
                lst = listDirectory(pathN);
                display(&start,&end,lst,&colNum);
                gotoxy(2,0);
                row=2;
            }
            else if(gotoDir(pathN,lst[start+row-2].path)==1){
                ioctlcall(&rowNum,&colNum);
                header(&colNum,"NORMAL MODE");
                lst.clear();
                strcat(pathN,"/");
                strcat(pathN,lst[start+row-2].path);
                string path = pathN;
                prev.push(path);
                while(!next.empty())
                    next.pop();
                lst = listDirectory(pathN);
                display(&start,&end,lst,&colNum);      
                gotoxy(2,0);
                row=2;
                start=0;
                end = rowNum-2;
            }
        }
        else if (c == KEY_UP){
            if(start>0 && row==rowNum-1){   
                start--;
                end--;
                header(&colNum,"NORMAL MODE");
                display(&start,&end,lst,&colNum);
                gotoxy(row,0);
            }
            else if(row>2){
                row--;
                cursorupward(1);
            }
        } 
        else if(c == KEY_DOWN) {
            if(row==rowNum-1 && start>=0 && end<lst.size()){
                start++;
                end++;
                header(&colNum,"NORMAL MODE");
                display(&start,&end,lst,&colNum);
                gotoxy(row,0);
            }
            else if(row<rowNum-1 && row<=lst.size()){
                row++;    
                cursordownward(1);
            }
        } 
        else if (c == KEY_RIGHT) {
            if(!next.empty()){
                ioctlcall(&rowNum,&colNum);
                header(&colNum,"NORMAL MODE");
                prev.push(next.top());
                next.pop();
                strcpy(pathN, prev.top().c_str());
                lst.clear();
                lst = listDirectory(pathN);
                display(&start,&end,lst,&colNum);
                gotoxy(2,0);
                row=2;
            }
        } 
        else if (c == KEY_LEFT) {
            if(prev.size()>1){
                ioctlcall(&rowNum,&colNum);
                header(&colNum,"NORMAL MODE");
                next.push(prev.top());
                prev.pop();
                strcpy(pathN, prev.top().c_str());
                lst.clear();
                lst = listDirectory(pathN);
                display(&start,&end,lst,&colNum);
                gotoxy(2,0);
                row=2;
            }
        }
        else if(c == BACKSPACE){
            if(strcmp(root,pathN)){
                ioctlcall(&rowNum,&colNum);
                header(&colNum,"NORMAL MODE");
                lst.clear();
                while(!next.empty())
                    next.pop();
                int i=0;
                string path = pathN;
                prev.push(path);
                for(i=strlen(pathN)-1;i>=0;i--){
                    if(pathN[i]=='/')
                        break;
                }
                pathN[i]='\0';
                path = pathN;
                prev.push(path);
                lst = listDirectory(pathN);
                display(&start,&end,lst,&colNum);
                gotoxy(2,0);
                row=2;
            }
        }
        if(c==':'){
            ioctlcall(&rowNum,&colNum);
            string str1 = commandmode(pathN, root, start, end);
            header(&colNum,"NORMAL MODE");
            fflush(stdout);
            cout.clear();
            while(!next.empty())
                next.pop();
            prev.push(str1);
            strcpy(pathN,str1.c_str());
            lst = listDirectory(pathN);
            display(&start,&end,lst,&colNum);
            gotoxy(2,0);
            row = 2;
        }
        else if(c == 'h' || c == 'H'){
            ioctlcall(&rowNum,&colNum);
            header(&colNum,"NORMAL MODE");
            memset(pathN,0,sizeof(pathN));
            strcpy(pathN,root);
            string path = pathN;
            while(!next.empty())
                next.pop();
            prev.push(path);
            lst = listDirectory(pathN);
            display(&start,&end,lst,&colNum);
            gotoxy(2,0);
            row=2;
            start=0;
            end = rowNum-2;
        }
        else if(c == 'q' || c == 'Q'){
                quitprogram();
        }
    }
    fclose(stderr);
    return 0;
}
