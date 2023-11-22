#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/wait.h>

void printXYSize(char *file, char* rez)
{
    int fd;
    fd = open (file, O_RDONLY);
    if (fd == -1)
    {
        perror("error open xy");
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

void makeGray(char *file)
{
    int fd;
    fd = open (file, O_RDWR);
    if (fd == -1)
    {
        perror("error open gray");
        exit (-1);
    }
    float gray;
    unsigned char rgb[3];
    lseek(fd, 54, SEEK_SET);
    for(int i = 0; i < 3; i++)
        if(read(fd, &rgb[i], sizeof(unsigned char)) != sizeof(unsigned char))
        {
            if(close(fd) ==  -1)
            {
                perror("error close rgb");
                exit(1);
            }
            perror("error read rgb");
            exit(1);
        }
    gray = 0.299 * rgb[0] + 0.587 * rgb[1] + 0.144 + rgb[2];
    printf("--------------%f\n", gray);
    
    lseek(fd, 54, SEEK_SET);
    for(int i = 0; i < 3; i++)
        if(write(fd, &gray, sizeof(unsigned char)) != 1)
        {
            if(close(fd) ==  -1)
            {
                perror("error close rgb");
                exit(1);
            }
            perror("error read rgb");
            exit(1);
        }
    
    if(close(fd) ==  -1)
    {
        perror("error close rgb");
        exit(1);
    }
    
}

void printStats(char* path, struct stat stats, int fd){
    char permisions[4];
    char printableString[1024];
    char *name = strchr(path, '/') + 1;
    sprintf(printableString, "Name: %s\n", name);
    if(strstr(name, ".bmp"))
    {
        char printableSize[128];
        printXYSize(path, printableSize);
        strcat(printableString, printableSize);
        pid_t pid;
        // int status;
        pid = fork();
        if(pid < 0)
        {
            perror("fork error");
            exit(-1);
        }
        else if(pid == 0)
        {
            makeGray(path);
            exit(1);
        }
        // else
        // {
        //     pid = wait(&status);
        //     if(WIFEXITED(status))
        //         printf("Child with pid %d, in photo graying process, ended with status %d\n", pid, WEXITSTATUS(status));
        // }
    }
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

void citire_director(char *director_intrare, char *director_iesire){
    DIR *dir;
    dir = opendir (director_intrare);

    if(dir == NULL){
        perror("open director_intrare");
        exit(1);
    }

    struct dirent *entry;
    struct stat stats;
    char path[300];
    pid_t pid;
    int status;
    while((entry = readdir(dir)) != NULL)
    {
        pid = fork();
        if(pid < 0)
        {
            perror("fork error");
            exit(-1);
        }
        else if(pid == 0)
        {
            if(strcmp(entry -> d_name, ".") != 0 && strcmp(entry -> d_name, "..") != 0)
            {
                char file[300];
                sprintf(file, "%s/%s_statistics.txt",director_iesire, entry -> d_name);
                // printf("%s\n", file);
                int fd;
                fd = open (file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
                if (fd == -1)
                {
                    perror("error open write");
                    exit (-1);
                }
                struct stat st;
                if(stat(file,&st) == -1)
                {
                perror("statistics.txt stat error");
                exit(1);
                }

                sprintf(path, "%s/%s", director_intrare, entry -> d_name);
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
                        printStats(path, stats, fd);
                }
                if(close(fd) == -1)
                {
                    perror("error close");
                    exit(1);
                }
            }
            exit(1);
        }

    }
    while( (pid = wait(&status)) != -1)
    {
        if(WIFEXITED(status))
            printf("Child with pid %d, ended with status %d\n", pid, WEXITSTATUS(status));
    }

    if(closedir(dir) == -1){
        perror("closed director_intrare");
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
    
    char *director_intrare = argv[1];
    char *directpr_iesire = argv[2];
    citire_director(director_intrare, directpr_iesire);
    return 0;
}