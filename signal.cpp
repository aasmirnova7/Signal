#include <stdio.h> //printf
#include <unistd.h> //chdir, _exit, fork, close, dup2, execl, read
#include <signal.h>
#include <fcntl.h> //creat, open
#include <sys/wait.h>
#include <setjmp.h>
#include <stdlib.h> // system
#include <sys/stat.h>

int SigCount = 0; //счётчик прерываний
sigjmp_buf obl; // область памяти для запоминания состояния процесса
/*
void a(int i){
   ++SigCount;
   printf("\nSIGNAL\n");
   signal(SIGINT, a);

   if(SigCount == 2)
   {
      printf("\nSIGNAL 2\n");
      system("du -c --block-size=512 `find -atime -5 -size +3`");
   }
   if(SigCount > 9)
       _exit(1);
   siglongjmp (obl, 1);    // возвращение на послед-ний     setjmp ,
                           // 2-арг - запоминает маску сигнала, если 2-ой аргумент больше 0
}
*/
void prer(int i){
   ++SigCount;
   printf("\nSIGNAL\n");

   if(SigCount == 2)
   {
      printf("\nSIGNAL 2\n");
      system("(du -c --block-size=512 `find -atime -5 -size +3`) | grep total");
   }
   if(SigCount > 9)
       _exit(1);
   siglongjmp (obl, 1);    // возвращение на послед-ний     setjmp
}


int main(int argc, char* argv[])
{
    char buf[5000] = {};
    int fd;
    int s; //статус процесса
    struct sigaction sa;  //спец. структура используемая в качестве параметра системного вызова sigaction()
    struct stat inf;
    if(sigprocmask(0,NULL, &sa.sa_mask) == -1)
    {
        printf("Ошибка получения сигнальной маски\n");
        exit(-1);
    }
    else {
        sa.sa_flags = 0;
        sa.sa_handler = prer; // добавляем в структуру типа  sigaction указатель на функцию обработчик сигнала
    }

    chdir("../");
    chdir(argv[1]);
    sigsetjmp (obl,1); // запоминание текущего состояния процесса
    //signal(SIGINT, a);
    sigaction(SIGINT, &sa, 0); /* этим вызовом с учётом всех предыдущих действий
                        мы назначили нашему процессу обработчик prer на случай получения сигнала SIGINT*/
    sleep(2);
    if(fork() == 0) //процесс сын
    {
        //sigsetjmp (obl,1); // запоминание текущего состояния процесса
        fd = creat("osi.txt", 0664);
        close(1);
        dup2(fd,1);
        close(fd);
        execl("/usr/bin/find", "find", "-atime", "-5", "-size", "+3", 0); //поиск файлов по дате последнего обращения
        //блок по 512б
    }
    else //процесс отец
    {
        //sigsetjmp (obl,1); // запоминание текущего состояния процесса
        wait(&s); //в s будет записан статус завершения процесса
        fd = open("osi.txt", 0);
        fstat(fd,&inf);
        read(fd, buf, inf.st_size);
        close(fd);
        printf("%s\n", buf);
    } 
}
