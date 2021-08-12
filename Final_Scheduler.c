  #include  <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>


typedef void (*fptr)();
struct task tasktoberemoved;
struct taskQueue {
	int CURRENTSIZE;
	int MAXSIZE;
	struct task *tasks;
};
struct taskQueue ready_queue;
struct taskQueue delay_queue;
 
struct task {
    int priority;
    fptr pointer_to_func;
    int delay;
};



bool flag=false;

bool test=true;

void  insertintodelay(fptr f, int delay, int prio){
    delay_queue.tasks[delay_queue.CURRENTSIZE].delay=delay+1;
    delay_queue.tasks[delay_queue.CURRENTSIZE].priority= prio;
    delay_queue.tasks[delay_queue.CURRENTSIZE].pointer_to_func =f;
    delay_queue.CURRENTSIZE++;
    
    for (int i=0; i<delay_queue.CURRENTSIZE;i++){
        for (int j=0; j<delay_queue.CURRENTSIZE;j++){
            if (delay_queue.tasks[i].delay < delay_queue.tasks[j].delay && i!=j){

                struct task temp = delay_queue.tasks[i];
	            delay_queue.tasks[i] = delay_queue.tasks[j];
	            delay_queue.tasks[j] = temp;
            }
        }
    }
}

void QueTask(fptr f, int Priority)
{
    ready_queue.tasks[ready_queue.CURRENTSIZE].priority=Priority;
    ready_queue.tasks[ready_queue.CURRENTSIZE].pointer_to_func =f;
    ready_queue.CURRENTSIZE++;

    for (int i=0; i<ready_queue.CURRENTSIZE;i++){
        for (int j=0; j<ready_queue.CURRENTSIZE;j++){
            if (ready_queue.tasks[i].priority > ready_queue.tasks[j].priority && i!=j){

                struct task temp = ready_queue.tasks[i];
	            ready_queue.tasks[i] = ready_queue.tasks[j];
	            ready_queue.tasks[j] = temp;
            }
        }
    }
    

}
void delayMs(int n){
    int i, j;
    for(i = 0 ; i < n; i++)
     for(j = 0; j < 3180; j++)
        {} // do nothing for 1 ms
   
}

void ReRunMe(int delay){
    if (delay==0) {
        QueTask(tasktoberemoved.pointer_to_func, tasktoberemoved.priority);
    }
    else {
        insertintodelay(tasktoberemoved.pointer_to_func, delay, tasktoberemoved.priority);

    }
}


void task3(struct taskQueue *q, int size){
    printf("T3\n");
    ReRunMe(3);
}

void print2(struct taskQueue *q, int size){
    printf("T2\n");
    ReRunMe(2);
}


void print(struct taskQueue *q, int size){
    printf("T1\n");
    ReRunMe(1);
}






void Nothing(){ 
}
 void init(int size)
{
    //printf("fgdgfd ");
    ready_queue.CURRENTSIZE=0;
    ready_queue.MAXSIZE=size;
    ready_queue.tasks = (struct task*)malloc(size * sizeof(struct task));

    delay_queue.CURRENTSIZE=0;
    delay_queue.MAXSIZE=size;
    delay_queue.tasks = (struct task*)malloc(size * sizeof(struct task));

    for(int i=0;i< size; i++)
        {
            ready_queue.tasks[i].priority=-1;
            ready_queue.tasks[i].delay=0;
            ready_queue.tasks[i].pointer_to_func = &Nothing;

            delay_queue.tasks[i].priority=-1;
            delay_queue.tasks[i].delay=99999;
            delay_queue.tasks[i].pointer_to_func = &Nothing;
        }  
} 
void swap()
{
    struct task temp=ready_queue.tasks[1];
    ready_queue.tasks[1]=ready_queue.tasks[0];
    ready_queue.tasks[0]=temp;
}

void dispatch(){
    
   
    if (ready_queue.CURRENTSIZE > 0){
    
        
        tasktoberemoved = ready_queue.tasks[0];

        for (int i=0;i<ready_queue.CURRENTSIZE;i++)
            ready_queue.tasks[i] = ready_queue.tasks[i+1];
        ready_queue.CURRENTSIZE--;
        ready_queue.tasks[ready_queue.CURRENTSIZE].priority = -1;
        ready_queue.tasks[ready_queue.CURRENTSIZE].delay=99999;
        ready_queue.tasks[ready_queue.CURRENTSIZE].pointer_to_func = &Nothing;

          
        tasktoberemoved.pointer_to_func();
        
         
        
          

         
    }
   else if (ready_queue.CURRENTSIZE==0)
        {
            printf("No task scheduled\n");
 
        }

    delayMs(50); //50ms after dispatch
    int i=0;
    while(delay_queue.CURRENTSIZE!=0)
            {
                if(delay_queue.tasks[i].delay== 99999)
                        break;
                if(delay_queue.tasks[0].delay!= 99999)//there is a task
                {
                   if (delay_queue.tasks[0].delay==1 && i==0)
                        delay_queue.tasks[0].delay--;
                    else if (delay_queue.tasks[0].delay>1 && i==0)
                        delay_queue.tasks[0].delay--;
                    else if (delay_queue.tasks[i].delay==1 && i!=0)
                        delay_queue.tasks[i].delay--;                 
                    else if (delay_queue.tasks[i].delay>1 && i!=0)
                        delay_queue.tasks[i].delay--;             
                }
                if(delay_queue.tasks[0].delay==0)
                    {
                        QueTask(delay_queue.tasks[0].pointer_to_func,delay_queue.tasks[0].priority);
                        
                        for (int i=0;i<delay_queue.CURRENTSIZE;i++)
                            delay_queue.tasks[i] = delay_queue.tasks[i+1];                       
                        delay_queue.CURRENTSIZE--;
                        delay_queue.tasks[delay_queue.CURRENTSIZE].priority = -1;
                        delay_queue.tasks[delay_queue.CURRENTSIZE].delay=99999;
                        delay_queue.tasks[delay_queue.CURRENTSIZE].pointer_to_func = &Nothing;
                        i--;
                    } 
                i++;                    
            }

    
}

int main(){
  init(20);
  QueTask(&print,6);
  QueTask(&print2,7); 
  QueTask(&task3,8);

  
  for (int p=0; p<10;p++){
   dispatch();
  }
  

  return 0;  
}

