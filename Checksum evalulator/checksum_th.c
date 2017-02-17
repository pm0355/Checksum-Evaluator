#include <sys/wait.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <assert.h>
#include <inttypes.h>


/*declare global variable to sum the xor*/
uint8_t sum8;

/*function declared to pass to threads to evaluate checksum in parallel*/
void *funct(void *argument)
{
   int passed_in_value = *((int *) argument);
   
    sum8+=passed_in_value;

  // printf("Hello World! It's me, thread with argument %d!\n", sum8);
 
   /* optionally: insert more useful stuff here */
 
   return NULL;
}

int main(int argc, char *argv[])
{
/*res will create each thread later but first let's instantiate it.*/
int res;
int NUMTHR =2* strtol(argv[2],NULL,10); // Number of parallel processes.

pthread_t thrs[NUMTHR]; //Number of threads instantiated in pthread array.

struct stat buf1; // Struct stat to store file information.
stat(argv[1], &buf1);
int input_size= buf1.st_size;
ssize_t numRead = 0;


int thr_size =input_size/NUMTHR; //size of each segmented to read to a buff.
int thr_size_r = input_size%NUMTHR; // remainder of div. if any.


int inFile = open(argv[1], O_RDONLY); // open file.
char inBuffer[input_size]; //create read buff.
char thr_buff[input_size];// create process buff.

int j_size=0;
int i =0;
int h=0;

//opens and reads file to buff, then computes checksum of each segmented passed to NUMTHR parallel processes.
while ((numRead = read(inFile, inBuffer, input_size)) != 0) {
    for(i=0; i<NUMTHR+thr_size_r; i++){
        inBuffer[i]^=inBuffer[i];
        res = pthread_create(&thrs[i], NULL, funct, (void *) &inBuffer[i+j_size]);
        if(j_size<(input_size - thr_size_r)){
        j_size+=thr_size;}
        if(j_size>=(input_size - thr_size_r)){
            j_size+=1;
        }
        assert(0==res); 
    }

    for(h=0; h<NUMTHR+thr_size_r;h++){
        res = pthread_join(thrs[h], NULL);
        assert(0==res);
    }
    

}
printf("%d\n",sum8);
}