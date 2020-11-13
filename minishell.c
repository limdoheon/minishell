/* minishell.c*/
/*
* 동의대학교 컴퓨터 소프트웨어 공학과
* 임도헌 , 김성은
*/
#include <stdio.h>
#include <stdlib.h> 
#include <unistd.h> 
#include <sys/types.h> 
#include <dirent.h> 
#include <sys/stat.h> 
#include <fcntl.h> 
#include <errno.h> 
#include <string.h> 
#include <signal.h> 
void my_ctrlc(int sig){ 
    signal(sig, SIG_IGN); 
    printf("  ctrl_C 입력 - 쉘 종료\n"); 
    exit(1); 
}
void my_ctrlz(int sig, int flag){
    signal(sig, SIG_IGN); 
    printf(" 쉘 일시정지..\n"); 
    printf(" fg 명령으로 재개 가능..\n"); 
    raise(SIGSTOP); 
    printf(" 쉘 재개..\n"); 
    signal(sig, my_ctrlz); 
}
int getargs(char *cmd, char **argv) {
    int narg =0; 
    while (*cmd) { 
        if (*cmd ==' '||*cmd =='\t') 
            *cmd ++='\0'; 
        else { 
            argv[narg++] = cmd ++; 
            while (*cmd !='\0'&&*cmd !=' '&&*cmd !='\t') 
                cmd++; 
        } 
    } 
    argv[narg] =NULL; 
    return narg;
}
int checkTaskOption(char *argv){  //실행 인자 포함 여부 확인 / 0=없음, -1 = &, 1 = pipe, 2 
//= <, 3 = > 
    int opt =0; 
    if(argv ==NULL){ 
        return opt; 
    } 
    for(int i =0; argv[i] !=NULL; i ++){ 
        if (argv[i] =='&'){ 
            opt =-1; 
            return opt; 
        } if (argv[i] =='|'){ 
            opt =1; 
            return opt; 
        } 
        if (argv[i] =='<'){ 
            opt =2; 
            return opt; 
        } 
        if (argv[i] =='>'){ 
            opt =3; 
            return opt; 
        } 
    } 
    
    return opt; 
}
void my_ls(){
    DIR *pdir; 
    struct dirent *pde; 
    int i =0; 
    if ( (pdir = opendir(".")) <0 ) { 
        perror("opendir"); 
        exit(1); 
    } 
    while ((pde = readdir(pdir)) !=NULL) { 
        printf("%20s ", pde ->d_name); 
        if (++i % 3 ==0) 
            printf("\n"); 
    } 
    printf("\n"); 
    closedir(pdir); 
}
void my_pwd(){
    char buf[1024]; 
    getcwd(buf, 1024); 
    printf("%s\n", buf); 
}
void pwd_print(){
    char buf[1024]; 
    getcwd(buf, 1024); 
    printf("%s", buf); 
}
void my_cd(char *path){
    if(chdir(path) <0){ 
        perror("chdir"); 
        exit(1); 
    } 
    else{ 
        printf("Move to "); 
        my_pwd(); 
    } 
}
void my_mkdir(char *path){
    if(mkdir(path, 0777) <0){ 
        perror("rmdir"); 
    }
} 
void my_rmdir(char *dir){ 
    if(rmdir(dir) <0){ 
        perror("rmdir"); 
    } 
}
void my_ln(char *src, char *target){
    if (link(src, target) <0){ 
        perror("link"); 
    } 
}
void my_cp(char *src, char *target){
    int src_fd; /* source file descriptor */ 
    int dst_fd; /* destination file descriptor */ 
    char buf[10]; 
    ssize_t rcnt; /* read count */ 
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; /* == 0644 */ 
    if ((src_fd = open(src, O_RDONLY)) ==-1 ){ 
        perror("src open"); /* errno에 대응하는 메시지 출력됨 */ 
        exit(1); 
    } 
    if ( (dst_fd = creat(target, mode)) ==-1 ){ 
        perror("dst open"); /* errno에 대응하는 메시지 출력됨 */ 
        exit(1); 
    } 
    while ( (rcnt = read(src_fd, buf, 10)) >0){ 
        write(dst_fd, buf, rcnt); 
    } 
    if (rcnt <0) { 
        perror("read"); 
        exit(1); 
    } 
    close(src_fd); 
    close(dst_fd); 
}
void my_rm(char *target){
    remove(target); 
}
void my_mv(char *file, char *path){
    my_cp(file, path); 
    my_rm(file); 
}
void my_cat(char *target){
    char buffer[512]; 
    int filedes; 
    /* target 을 읽기 전용으로 개방 */ 
    if ( (filedes = open (target, O_RDONLY) ) ==-1) 
    { 
        printf("error in opening anotherfile\n"); 
        exit (1); 
    } 
    /* EOF 까지 반복하라. EOF 는 복귀값 0 에 의해 표시된다 */ 
    while (read (filedes, buffer, 512) >0){ 
        printf("%s", buffer); 
    } 
}
void your_cat(int target){ 
    char buffer[512]; 
    while (read (target, buffer, 512) >0){ 
        printf("%s", buffer); 
    } 
}
void selectCmd(int i, char **argv){
    //argv 판별해서 맞는 명령 실행 
    if(!strcmp(argv[i], "cat")){ 
        if(argv[i +1] ==NULL){ 
            fprintf(stderr, "A few argument..!\n"); 
        } 
        int arg = argv[i +1][0] -48; 
        if( arg ==3 ) { 
            your_cat(arg); 
        } 
        else{ 
            my_cat(argv[i+1]); 
        } 
    } 
    else if(!strcmp(argv[i], "ls")){ 
        my_ls(); 
    } 
    else if(!strcmp(argv[i], "pwd")){ 
        my_pwd(); 
    } 
    else if(!strcmp(argv[i], "mkdir")){ 
        if(argv[i +1] ==NULL){ 
            fprintf(stderr, "A few argument..!\n"); 
        } 
        else{ 
            my_mkdir(argv[i+1]); 
        } 
    } 
    else if(!strcmp(argv[i], "rmdir")){ 
        if(argv[i +1] ==NULL){ 
            fprintf(stderr, "A few argument..!\n"); 
        } 
        else{ 
            my_rmdir(argv[i+1]); 
        } 
    } 
    else if(!strcmp(argv[i], "ln")){ 
        if(argv[i +1] ==NULL || argv[i +2] ==NULL){ 
            fprintf(stderr, "A few argument..!\n"); 
        } 
        else{ 
            my_ln(argv[i+1], argv[i +2]); 
        } 
    } 
    else if(!strcmp(argv[i], "cp")){ 
        if(argv[i +1] ==NULL || argv[i +2] ==NULL){ 
            fprintf(stderr, "A few argument..!\n"); 
        } 
        else{ 
            my_cp(argv[i+1], argv[i +2]); 
        } 
    }
    else if(!strcmp(argv[i], "rm")){ 
        if(argv[i +1] ==NULL){ 
            fprintf(stderr, "A few argument..!\n"); 
        } 
        else{ 
            my_rm(argv[i+1]); 
        } 
    } 
    else if(!strcmp(argv[i], "mv")){ 
        if(argv[i +1] ==NULL || argv[i +2] ==NULL){ 
            fprintf(stderr, "A few argument..!\n"); 
        } 
        else{ 
            my_mv(argv[i+1], argv[i +2]); 
        } 
    } 
    else{} 
}
void run(int i, int t_opt, char **argv){
    pid_t pid; 
    int fd; /* file descriptor */ 
    char *buf[1024]; 
    int flags = O_RDWR | O_CREAT; 
    mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH; /* == 0644 */ 
    memset(buf, 0, 1024); 
    pid = fork(); 
    if (pid ==0){  //child 
        //-1 = &, 1 = pipe, 2 = <, 3 = > 
        if(t_opt ==0){ 
            selectCmd(i, argv); 
            exit(0); 
        } 
        else if(t_opt ==2){ 
            if ((fd = open(argv[i +2], flags, mode)) ==-1) { 
                perror("open"); /* errno에 대응하는 메시지 출력됨*/ 
                exit(1); 
            } 
            if (dup2(fd, STDIN_FILENO) ==-1) { 
                perror("dup2"); /* errno에 대응하는 메시지 출력됨 */ 
                exit(1); 
            } 
            if (close(fd) ==-1) { 
                perror("close"); /* errno에 대응하는 메시지 출력됨*/ 
                exit(1); 
            } 
            my_cat(argv[i+2]); 
            selectCmd(i, argv); 
            exit(0); 
        } 
        else if(t_opt ==3){ 
            if ((fd = open(argv[i +2], flags, mode)) ==-1) { 
                perror("open"); /* errno에 대응하는 메시지 출력됨*/ 
                exit(1); 
            }if (dup2(fd, STDOUT_FILENO) ==-1) { 
                perror("dup2"); /* errno에 대응하는 메시지 출력됨 */ 
                exit(1); 
            } 
            if (close(fd) ==-1) { 
                perror("close"); /* errno에 대응하는 메시지 출력됨*/ 
                exit(1); 
            } 
            selectCmd(i, argv); 
            exit(0); 
        } 
    } 
    else if (pid >0){  //parent - 백그라운드 아닐 때만 기다림 
        if(t_opt >=0){ //백그라운드가 아닐 때 
            wait(pid);
    } 
        if(!strcmp(argv[i], "cd")){ 
            if(argv[i +1] ==NULL){ 
                fprintf(stderr, "A few argument..!\n"); 
            } 
            else{ 
                my_cd(argv[i+1]); 
            } 
        } 
    } 
    else{ 
        perror("fork failed"); 
    }
}
void run_pipe(int i, char **argv){
    char buf[1024]; 
    int p[2]; 
    int pid; 
    /* open pipe */ 
    if (pipe(p) ==-1) { 
        perror ("파이프 연결 실패"); 
        exit(1); 
    } 
    pid = fork(); 
    if (pid ==0) { /* child process */ 
        close(p[0]); 
        if (dup2(p[1], STDOUT_FILENO) ==-1) { 
            perror("dup2"); /* errno에 대응하는 메시지 출력됨 */ 
            exit(1); 
        } 
        close(p[1]); 
        selectCmd(i, argv); 
        exit(0); 
    } 
    else if (pid >0) { 
        wait(pid); 
        char *arg[1024]; 
        close(p[1]); 
        sprintf(buf, "%d", p[0]); 
        arg[0] = argv[i +2]; 
        arg[1] = buf; 
        selectCmd(0, arg); 
    }else 
        perror ("fork failed"); 
}
void main() {
    char buf[256]; 
    char *argv[50]; 
    int narg; 
    struct sigaction ctrlc_act; 
    struct sigaction ctrlz_act; 
    ctrlc_act.sa_handler = my_ctrlc; 
    ctrlz_act.sa_handler = my_ctrlz; 
    sigaction(SIGINT, &ctrlc_act, NULL); 
    sigaction(SIGTSTP, &ctrlz_act, NULL); 
    printf("미니쉘을 시작합니다!\n"); 
    while (1) { 
        pwd_print(); 
        printf(" : 쉘> "); 
        gets(buf); 
        narg = getargs(buf, argv);  //들어온 인자 갯수 
        int t_opt =0;  //task option 
        for (int i =0; i < narg; i ++) { 
            if(!strcmp(argv[i], "exit")){ 
                printf("쉘을 종료합니다../\n"); 
                exit(1); 
            } 
            int t_opt = checkTaskOption(argv[i +1]);    //-1 = &, 1 = pipe, 2 = <, 3 = > 
            if(t_opt ==1){ 
                run_pipe(i, argv); 
                i +=2; 
            } 
            else{ 
                run(i, t_opt, argv); 
            } 
            if(t_opt >1){  //it's optional arg 
                i +=2; 
            } 
        }   //end for loop 
    }   //end while loop 
}
