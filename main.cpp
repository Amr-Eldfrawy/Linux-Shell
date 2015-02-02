#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/types.h>
#include <signal.h>
#include <sys/wait.h>

#define MAX  1000
char *path[8];
char splitedLine[MAX][MAX];
char* vaildPath;
char command_path[MAX];
char history[1024][1000];
int history_index=0;
FILE *fp;


void buildPaths()
{

    path[0]="/usr/bin/";
    path[1]="/usr/lib/lightdm/lightdm/";
    path[2]="/usr/local/sbin/";
    path[3]="/usr/local/bin/";
    path[4]="/usr/sbin/";
    path[5]="/sbin/";
    path[6]="/bin/";
    path[7]="/usr/games/";
}

int getCommandPath(char*command)
{
    int res=-1;
    int i;
    vaildPath=NULL;

    for( i= 0 ; i < 8; i++)
    {
        memset(command_path,0,sizeof(command_path));

        strcpy(command_path,path[i]);
        strcat(command_path,command);


        if(access(command_path,F_OK)==0)
        {

            vaildPath=command_path;
            res = i;
            break;
        }

    }

    return res;

}


int split(char * line)
{
    int i,z;
    for(i=0; i<MAX; i++)
    {
        for(z=0; z<MAX; z++)
        {
            splitedLine[i][z]=0;
        }
    }
    int size=0;
    char temp[MAX];
    memset(temp,0,sizeof(temp));
    int j=0;

    while( *line!='\0' )
    {
        if(*line==' ' &&*(line-1)!='\\')
        {
            strcpy(splitedLine[size++],temp);


            j=0;
            memset(temp,0,sizeof(temp));
        }
        else
            temp[j++]=*line;

        line++;
    }

    if(j>0)
    {

        strcpy(splitedLine[size++],temp);
        j=0;

    }
    return size;
}
void wirteInLogFile(char *fileName,int command)
{
    fprintf(fp,"Child process was terminated with ID %d\n",command);
}
void handler(int sig)
{
    pid_t pid;

    pid = wait(NULL);
    wirteInLogFile("a.out",pid);
    return ;
}
void print_history()
{
    int i;

    for(i=0; i<history_index; i++)
    {
        printf("%s\n",history[i]);
    }
}
void do_fork(char* path,char* argv[], int mode)
{
    signal(SIGCHLD, handler) ;
    pid_t pid = fork();

    if (pid==0)
    {
        execv(path, argv);
        exit(1);

    }
    else if ( pid < 0 )
    {
        printf("failed to fork !\n");
        exit(1);
    }
    else
    {
        if(mode==0)
        {
            int status = 0;
            pid_t wpid ;

            while ((wpid = wait(&status))>0);
        }
        else
        {
            waitpid(pid,0,WNOHANG);
        }


    }

}
void execute(char * lineCommand)
{
    int len =split(lineCommand);
    int i;
    int mode = 0;

    if(strcmp(splitedLine[len-1],"&")==0)
        len--,mode=1;

    char* P[len+1];
    memset(P,0,sizeof(P));
    for(i=0 ; i < len; i++)
    {
        P[i]=splitedLine[i];
        puts(P[i]);
    }

    P[len]=NULL;

    if(strcmp(P[0],"cd")==0)
    {
        puts(P[0]);

        if(P[1]!=NULL)
        {
            chdir(P[1]);
        }
        else
        {
            chdir("..");
        }
    }
    else
    {
        getCommandPath(P[0]);
        do_fork(vaildPath,P,mode);
    }
}

void runFromBatch(char * fileName)
{
    char ch,filePath[MAX];
    FILE *fp;
    strcpy(filePath,fileName);

    fp = fopen(filePath,"r"); // read mode

    if( fp == NULL )
    {
        perror("Error while opening the file.\n");

        return ;
    }

    printf("The contents of %s file are :\n", filePath);
    size_t len = 0;
    char line[MAX];
    int j=0;

    memset(line,0,sizeof(line));
    char c;
    char *read;

    while( (c = getc(fp)) != EOF)
    {
        if(c=='\n')
        {
            j=0;
            read = line;
            printf("%s\n",line);
            execute(read);
            memset(line,0,sizeof(line)) ;
        }
        else
        {
            line[j++]=c;
        }

    }





}
char arr[MAX];
int main()
{

    fp = fopen("a.out","w");
    buildPaths();

    while(1)
    {
        char *p;

        memset(arr,0,sizeof(arr));
        printf("SHELL >>");
        gets(arr);
        strcpy(history[history_index],arr);
        history_index++;
        if(strcmp(arr,"history")==0)
        {
            print_history();
        }
        else if(strcmp(arr,"exit")==0)
        {
            break;
        }
        p=arr;
        execute(p);
        p=NULL;
    }

    fclose(fp);
    return 0;
}
