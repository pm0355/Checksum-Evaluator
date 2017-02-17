#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>

#define READ  0
#define WRITE 1

int main(int argc, char *argv[])
{
    //instantiate process variables to store pid and wait() var.
    pid_t cpid, w;
    int status;
    //instantiate var to iterate through loops
    int i=0;
    int j=0;
    int h=0;
    int loop_iter=0;
    int loop_count=0;
    // number of parallel processes
    int pcount=strtol(argv[2],NULL,10);
   
    struct stat buf1; // Struct stat to store file information.
    stat(argv[1], &buf1);
    int input_size= buf1.st_size;
    ssize_t numRead = 0;


    int exit_c =0;
    int ppcount=pcount-1;
    int inFile = open(argv[1], O_RDONLY);
    
    //size of each segmented to read to a buff.
    char inBuffer[input_size]; 
    char p_buff[input_size];
    int p_size = 2*(input_size/pcount);
    int p_size_r =input_size%pcount;
    
    uint8_t sum8;
    
    //instantiate pipe vars, to set up pipe between processes.
    int mypipefd[2];
    int mypipefd2[2];
    int checks =0;
    
    //creates pipe to read and write information between processes.
    if (pipe(mypipefd)== -1) {
    fprintf(stderr,"Pipe failed");
    return 1;
  }
  
  //parallel process decision tree. inefficient, yes.
    if (pcount==8)
        loop_iter=4;
    if (pcount==4)
        loop_iter=3;
    if (pcount==2)
        loop_iter=2;
    if (pcount==1)
        loop_iter=1;
        
//opens and reads file to buff, then computes checksum of each segmented passed to parallel processes.
    while ((numRead = read(inFile, inBuffer, input_size)) != 0) {
        for(h=0;h<p_size_r;h++){
                p_buff[h] ^=inBuffer[input_size-p_size_r+h+1];
                sum8+=p_buff[input_size-p_size_r+h+1];
            }
//forks new child process.
    for(i=0;i<loop_iter;i++){
        cpid=fork();
        
        if (cpid==0){ //child process
            //eval chks segments into buff
            int psum=0;

            for(j=0; j<p_size; j++){    
                p_buff[j] ^=inBuffer[j+loop_count];
                psum+=p_buff[j+loop_count];
            }
            loop_count+=p_size;
         //   printf("child, pid: %d\n", getpid());
    
        /* pipes to mypipe[READ] to var*/
            close(mypipefd[READ]);
            write(mypipefd[WRITE],&psum, sizeof(psum));    //write to pipe one
            close(mypipefd[WRITE]);
        }
        else{ 
        //parent process, wait exit status xor
            exit_c++;
            int sum=0;
            close(mypipefd[WRITE]);   //close write end, read, and then close read end
            read(mypipefd[READ],&sum,sizeof(sum));
            close(mypipefd2[READ]);
       

            
            w = waitpid(-1, &status, 0);
            if (w == -1) {
                perror("waitpid");
            }
            
            if (WIFEXITED(status)) {
            sum8+=sum;
            write(mypipefd2[WRITE],&sum8,sizeof(sum8));
          // printf("exited, status=%d\n", WEXITSTATUS(status));
            close(mypipefd2[WRITE]);
            }
            if(exit_c==loop_iter)
                printf("%d\n",sum8 );
            }
        if(cpid<0){
        printf("error in fork: %d", -1);
    
        }
        
        }
     
    }
return 0;
}
