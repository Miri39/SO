#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void printXYSize(char *f, char* rez)
{
    char *file = f;
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
    char buff[32];
    sprintf(rez, "inaltime %d\n", inaltimea);
    sprintf(buff, "lungimea %d\n", lungimea);
    strcat(rez, buff);

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

void printStats(char* name, struct stat stats, int fd){
    char permisions[4];
    char printableString[1024];
    sprintf(printableString, "Name: %s\n", name);
    if(strstr(name, ".bmp"))
    {
        char printableSize[128];
        printXYSize(name, printableSize);
        strcat(printableString, printableSize);
    }
    // write(fd, &stats.st_size, sizeof(long int));
    char buff[128];
    sprintf(buff, "Dimensiune: %ld\n", stats.st_size);
    strcat(printableString, buff);
    sprintf(buff, "User: %d\n", stats.st_uid);
    strcat(printableString, buff);
    sprintf(buff, "Last Modification: %s", ctime(&stats.st_mtime));
    strcat(printableString, buff);
    sprintf(buff, "Legaturi: %ld\n", stats.st_nlink);
    strcat(printableString, buff);
    sprintf(buff, "Permisiuni User: %s\n", (getUserPermisions(permisions,stats)));
    strcat(printableString, buff);
    sprintf(buff, "Permisiuni Group: %s\n", (getGroupPermisions(permisions,stats)));
    strcat(printableString, buff);
    sprintf(buff, "Permisiuni Others: %s\n", (getOthersPermisions(permisions,stats)));
    strcat(printableString, buff);
    sprintf(buff, "\n");
    strcat(printableString, buff);
    if(write(fd,printableString, strlen(printableString)) == -1)
    {
      perror("error write");
      exit(1);
    }
}

void printStatsLeg(char* name, struct stat stats, struct stat lstats, int fd){
    char permisions[4];
    char printableString[1024];

    sprintf(printableString, "Numele: %s\n", name);

    char buff[128];

    sprintf(buff, "Dimensiune fisier: %ld\n", stats.st_size);
    strcat(printableString, buff);
    sprintf(buff, "Dimensiune legatura: %ld\n", lstats.st_size);
    strcat(printableString, buff);
    sprintf(buff, "Permisiuni User: %s\n", (getUserPermisions(permisions,stats)));
    strcat(printableString, buff);
    sprintf(buff, "Permisiuni Group: %s\n", (getGroupPermisions(permisions,stats)));
    strcat(printableString, buff);
    sprintf(buff, "Permisiuni Others: %s\n", (getOthersPermisions(permisions,stats)));
    strcat(printableString, buff);
    sprintf(buff, "\n");
    strcat(printableString, buff);

    if(write(fd, printableString, strlen(printableString)) == -1)
    {
      perror("error write");
      exit(1);
    }
}

void printStatsDir(char* name, struct stat stats, int fd){
    char permisions[4];
    char printableString[1024];

    sprintf(printableString, "Numele: %s\n", name);

    char buff[128];
    sprintf(buff, "User: %d\n", stats.st_uid);
    strcat(printableString, buff);
    sprintf(buff, "Permisiuni User: %s\n", (getUserPermisions(permisions,stats)));
    strcat(printableString, buff);
    sprintf(buff, "Permisiuni Group: %s\n", (getGroupPermisions(permisions,stats)));
    strcat(printableString, buff);
    sprintf(buff, "Permisiuni Others: %s\n", (getOthersPermisions(permisions,stats)));
    strcat(printableString, buff);
    sprintf(buff, "\n");
    strcat(printableString, buff);

    if(write(fd, printableString, strlen(printableString)) == -1)
    {
      perror("error write");
      exit(1);
    }
}

void citire_director(char *director){
    DIR *dir;
    dir = opendir (director);
    char *file = "statistics.txt";
    struct stat st;
    if(stat(file,&st) == -1)
    {
      perror("statistics.txt stat error");
      exit(1);
    }
    int fd;
    fd = open (file, O_WRONLY);
    if (fd == -1)
    {
        perror("error open write");
        exit (-1);
    }

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
            if( entry->d_type == DT_LNK)
            {
                struct stat lstats;
                if(lstat(path, &lstats) == -1)
                {
                    perror("err stat");
                    exit(1);
                }
                printStatsLeg(entry -> d_name, stats, lstats, fd);
            }
            else
            {
                if( S_ISDIR(stats.st_mode) )
                    printStatsDir(entry -> d_name, stats, fd);
                else
                    printStats(entry -> d_name, stats, fd);
            }
        }
    }

    if(closedir(dir) == -1){
        perror("closed director");
        exit(1);
    }
    if(close(fd) == -1)
    {
        perror("error close");
        exit(1);
    }
    
}

int main (int argc, char *argv[])
{
    if(argc != 2)
    {
        perror("wrong number of arguments");
        exit(1);
    }
    
    char *director = argv[1];
    citire_director(director);
    return 0;
}