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

#define BMP_FILE strstr(name, ".bmp")
#define NOT_BMP_FILE !strstr(name, ".bmp")

char containedCharacter[2];
int sumFromScript = 0;

typedef struct xySize{
    int lungime;
    int inaltime;
}xySize;

xySize getXYSize(char *file)
{
    xySize xy;
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
    if(close(fd) ==  -1)
    {
        perror("error close");
        exit(1);
    }
    xy.inaltime = inaltimea;
    xy.lungime = lungimea;
    return xy;
}

void printXYSize(char *file, char* rez)
{
    xySize xy = getXYSize(file);

    char buff[32];
    sprintf(rez, "inaltime %d\n", xy.inaltime);
    sprintf(buff, "lungimea %d\n", xy.lungime);
    strcat(rez, buff);
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

void makeGray(char *file, int size)
{
    int fd;
    fd = open (file, O_RDWR);
    if (fd == -1)
    {
        perror("error open gray");
        exit (-1);
    }
    lseek(fd, 54, SEEK_SET);
    unsigned char buffer[3 * size];
    int bytes;
    if((bytes = read(fd, buffer, sizeof(buffer))) != 3 * size)
    {
        if(close(fd) ==  -1)
        {
            perror("error close rgb\n");
            exit(1);
        }
        perror("error read rgb\n");
        exit(1);
    }
    for(int pixel = 0; pixel < 3 * size; pixel = pixel + 3)
    {
        unsigned char gray = 0.299 * buffer[pixel] + 0.587 * buffer[pixel + 1] + 0.144 + buffer[pixel + 2];
        buffer[pixel] = gray;
        buffer[pixel + 1] = gray;
        buffer[pixel + 2] = gray;
    }
    lseek(fd, 54, SEEK_SET);
    if(write(fd, buffer, 3 * size) != 3 * size)
    {
        printf("da");
        if(close(fd) ==  -1)
        {
            perror("error close rgb\n");
            exit(1);
        }
        perror("error write rgb\n");
        exit(1);
    }
    if(close(fd) ==  -1)
    {
        perror("error close rgb");
        exit(1);
    }
}

void printStats(char* path, struct stat stats, int fd){
    pid_t pid;
    int contor = 0;
    char permisions[4];
    char printableString[1024];
    char *name = strchr(path, '/') + 1;
    int size = 0;
    int pfdToScript[2];
    int pfdFromScript[2];
    if(pipe(pfdToScript) < 0)
    {
        perror("error pipe");
        exit(-1);
    }
    pid = fork();
    if(pid < 0)
    {
        perror("fork error");
        exit(-1);
    }
    if(pid == 0)
    {
        sprintf(printableString, "Name: %s\n", name);
        contor++;
        if(BMP_FILE)
        {
            close(pfdToScript[0]);
            close(pfdToScript[1]);
            char printableSize[128];
            printXYSize(path, printableSize);
            strcat(printableString, printableSize);
            contor += 2;
        }
        char buff[128];
        sprintf(buff, "Dimensiune: %ld\n", stats.st_size);
        strcat(printableString, buff);
        contor++;

        sprintf(buff, "User: %d\n", stats.st_uid);
        strcat(printableString, buff);
        contor++;

        sprintf(buff, "Last Modification: %s", ctime(&stats.st_mtime));
        strcat(printableString, buff);
        contor++;
        sprintf(buff, "Legaturi: %ld\n", stats.st_nlink);
        strcat(printableString, buff);
        contor++;
        sprintf(buff, "Permisiuni User: %s\n", (getUserPermisions(permisions,stats)));
        strcat(printableString, buff);
        contor++;
        sprintf(buff, "Permisiuni Group: %s\n", (getGroupPermisions(permisions,stats)));
        strcat(printableString, buff);
        contor++;
        sprintf(buff, "Permisiuni Others: %s\n", (getOthersPermisions(permisions,stats)));
        strcat(printableString, buff);
        contor++;

        if(write(fd,printableString, strlen(printableString)) == -1)
        {
        perror("error write");
        exit(1);
        }
        if(NOT_BMP_FILE)
        {
            close(pfdToScript[0]); //inchidem capatul de citire
            char text[200];
            int fd;
            fd = open (path, O_RDONLY);
            if (fd == -1)
            {
                perror("error open read");
                exit (-1);
            }
            int bytesRead;
            while((bytesRead = read(fd, &text, sizeof(text))) > 0)
                if(write(pfdToScript[1], text, bytesRead) == -1)
                {
                    perror("error write pipe");
                    exit(-1);
                }
            close(pfdToScript[1]);
            close(fd);
        }
        exit(contor);
    }
    else
    {
        if(BMP_FILE)
        {
            pid = fork();
            if(pid < 0)
            {
                perror("fork error");
                exit(-1);
            }
            else if(pid == 0)
            {
                xySize xy = getXYSize(path);
                size = xy.inaltime * xy.lungime;
                makeGray(path, size);
                exit(0);
            }
        }
    }
    if(NOT_BMP_FILE)
    {
        if(pipe(pfdFromScript) < 0)
        {
            perror("error pipe");
            exit(-1);
        }
        pid = fork();
        if(pid < 0)
        {
            perror("fork error");
            exit(-1);
        }
        if(pid == 0)
        {
            close(pfdToScript[1]);
            close(pfdFromScript[0]);

            dup2(pfdToScript[0], 0);
            close(pfdToScript[0]);
            dup2(pfdFromScript[1],1);
            close(pfdFromScript[1]);
            execlp("bash", "bash", "rewrite", containedCharacter, NULL); //datorita modului in care e facut scriptul, o linie care se va rermina cu EOF in loc de \n, nu va fi procesata
            perror("erroare execlp");
            exit(-1);
        }
        close(pfdToScript[0]);
        close(pfdToScript[1]);
        close(pfdFromScript[1]);

        char buff[6];
        read(pfdFromScript[0], buff, sizeof(buff));
        sumFromScript += atoi(buff);
        close(pfdFromScript[0]);
    }
}

void printStatsLeg(char* name, struct stat stats, struct stat lstats, int fd){
    pid_t pid;
    int contor = 0;
    pid = fork();
    if(pid < 0)
    {
        perror("fork error");
        exit(-1);
    }
    if(pid == 0)
    {
        char permisions[4];
        char printableString[1024];

        sprintf(printableString, "Numele: %s\n", name);
        contor++;

        char buff[128];

        sprintf(buff, "Dimensiune fisier: %ld\n", stats.st_size);
        strcat(printableString, buff);
        contor++;

        sprintf(buff, "Dimensiune legatura: %ld\n", lstats.st_size);
        strcat(printableString, buff);
        contor++;

        sprintf(buff, "Permisiuni User: %s\n", (getUserPermisions(permisions,stats)));
        strcat(printableString, buff);
        contor++;

        sprintf(buff, "Permisiuni Group: %s\n", (getGroupPermisions(permisions,stats)));
        strcat(printableString, buff);
        contor++;

        sprintf(buff, "Permisiuni Others: %s\n", (getOthersPermisions(permisions,stats)));
        strcat(printableString, buff);
        contor++;

        if(write(fd, printableString, strlen(printableString)) == -1)
        {
        perror("error write leg");
        exit(1);
        }
        exit(contor);
    }
}

void printStatsDir(char* name, struct stat stats, int fd){
    pid_t pid;
    int contor = 0;
    pid = fork();
    if(pid < 0)
    {
        perror("fork error");
        exit(-1);
    }
    if(pid == 0)
    {
        char permisions[4];
        char printableString[1024];

        sprintf(printableString, "Numele: %s\n", name);
        contor++;

        char buff[128];
        sprintf(buff, "User: %d\n", stats.st_uid);
        strcat(printableString, buff);
        contor++;
        
        sprintf(buff, "Permisiuni User: %s\n", (getUserPermisions(permisions,stats)));
        strcat(printableString, buff);
        contor++;

        sprintf(buff, "Permisiuni Group: %s\n", (getGroupPermisions(permisions,stats)));
        strcat(printableString, buff);
        contor++;

        sprintf(buff, "Permisiuni Others: %s\n", (getOthersPermisions(permisions,stats)));
        strcat(printableString, buff);
        contor++;

        if(write(fd, printableString, strlen(printableString)) == -1)
        {
            perror("error write dir");
            exit(1);
        }
        exit(contor);
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
            if(strcmp(entry -> d_name, ".") != 0 && strcmp(entry -> d_name, "..") != 0)
            {
                char file[300];
                sprintf(file, "%s/%s_statistics.txt",director_iesire, entry -> d_name);
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
    if(argc != 4)
    {
        perror("wrong number of arguments");
        exit(-1);
    }
    if(strlen(argv[3]) != 1)
    {
        perror("not a char in command line arg");
        exit(-1);
    }
    char *director_intrare = argv[1];
    char *directpr_iesire = argv[2];
    strcpy(containedCharacter,argv[3]);
    citire_director(director_intrare, directpr_iesire);
    printf("Au fost identificate in total %d propozitii corecte care contin caracterul %c\n", sumFromScript, argv[3][0]);
    return 0;
}