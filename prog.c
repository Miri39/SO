#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void printXYSize(char *f)
{
    char *file = f;
    // struct stat buf;
    // if(stat(file,buf) == -1)
    // {

    // }
    int fd;
    fd = open (file, O_RDONLY);
    if (fd == -1)
    {
        perror("error open");
        exit (-1);
    }
    int inaltimea, lungimea;
    lseek(fd, 18, SEEK_SET);
    if(read(fd, &inaltimea, sizeof(int)) != sizeof(int))
    {
        if(close(fd) ==  -1)
        {
            perror("error close");
            exit(1);
        }
        perror("error read");
        exit(1);
    }
    if(read(fd, &lungimea, sizeof(int)) != sizeof(int))
    {
        if(close(fd) ==  -1)
        {
            perror("error close");
            exit(1);
        }
        perror("error read");
        exit(1);
    }
    printf("inaltime %d\n", inaltimea);
    printf("lungimea %d\n", lungimea);
    if(close(fd) ==  -1)
    {
        perror("error close");
        exit(1);
    }
}

char* getUserPermisions(char* permisions, struct stat stats){
    if(stats.st_mode & S_IRUSR)
        permisions[0] = 'R';
    else
        permisions[0] = '-';
    if(stats.st_mode & S_IWUSR)
        permisions[1] = 'W';
    else
        permisions[1] = '-';
    if(stats.st_mode & S_IXUSR)
        permisions[2] = 'X';
    else
        permisions[2] = '-';
    permisions[3] = '\0';
    return permisions;
}

char* getGroupPermisions(char* permisions, struct stat stats){
    if(stats.st_mode & S_IRGRP)
        permisions[0] = 'R';
    else
        permisions[0] = '-';
    if(stats.st_mode & S_IWGRP)
        permisions[1] = 'W';
    else
        permisions[1] = '-';
    if(stats.st_mode & S_IXGRP)
        permisions[2] = 'X';
    else
        permisions[2] = '-';
    permisions[3] = '\0';
    return permisions;
}

char* getOthersPermisions(char* permisions, struct stat stats){
    if(stats.st_mode & S_IROTH)
        permisions[0] = 'R';
    else
        permisions[0] = '-';
    if(stats.st_mode & S_IWOTH)
        permisions[1] = 'W';
    else
        permisions[1] = '-';
    if(stats.st_mode & S_IXOTH)
        permisions[2] = 'X';
    else
        permisions[2] = '-';
    permisions[3] = '\0';
    return permisions;
}

void printStats(char* name, struct stat stats){
    char permisions[4];

    printf("Numele: %s\n", name);
    if(strstr(name, ".bmp"))
    {
        printXYSize(name);
    }
    printf("Dimensiune: %ld\n", stats.st_size);
    printf("User: %d\n", stats.st_uid);
    printf("Last Modification: %s", ctime(&stats.st_mtime));
    printf("Legaturi: %ld\n", stats.st_nlink);
    printf("Permisiuni User: %s\n", (getUserPermisions(permisions,stats)));
    printf("Permisiuni Group: %s\n", (getGroupPermisions(permisions,stats)));
    printf("Permisiuni Others: %s\n", (getOthersPermisions(permisions,stats)));
    printf("\n");
}

void printStatsDir(char* name, struct stat stats){
    char permisions[4];

    printf("Numele: %s\n", name);
    printf("User: %d\n", stats.st_uid);
    printf("Permisiuni User: %s\n", (getUserPermisions(permisions,stats)));
    printf("Permisiuni Group: %s\n", (getGroupPermisions(permisions,stats)));
    printf("Permisiuni Others: %s\n", (getOthersPermisions(permisions,stats)));
    printf("\n");
}

void citire_director(char *director){
    DIR *dir;
    dir = opendir (director);

    if(dir == NULL){
        perror("open director");
        exit(1);
    }

    struct dirent *entry;
    struct stat stats;
    char path[300];
    while((entry = readdir(dir)) != NULL)
    {
        if(strcmp(entry -> d_name, ".") != 0 && strcmp(entry -> d_name, "..") != 0)
        {
            sprintf(path, "%s/%s", director, entry -> d_name);
            if(stat(path, &stats) == -1)
            {
                perror("err stat");
                exit(1);
            }
            if( S_ISLNK(stats.st_mode) )
            {
                if(lstat(path, &stats) == -1)
                {
                    perror("err stat");
                    exit(1);
                }
            }
            if( S_ISDIR(stats.st_mode) )
                printStatsDir(entry -> d_name, stats);
            else
                printStats(entry -> d_name, stats);
            
        }
    }

    if(closedir(dir) == -1){
        perror("closed director");
        exit(1);
    }
    
}

int main (int argc, char *argv[])
{
    if(argc != 3)
    {
        perror("wrong number of arguments");
        exit(1);
    }
    
    char *director = argv[2];
    citire_director(director);
    return 0;
}