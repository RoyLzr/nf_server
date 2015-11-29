#include "../../NfUnit/eventqueue.h"
#include "../../interface/ievent.h"
#include "stdio.h"
#include "stdlib.h"

class TestEvent : public IEvent
{
    public:
          TestEvent() {};
          TestEvent(int num) : label(num)  {};
          int handle() { return 0;}
        
          void setHandle(int) {return;}

          IReactor *reactor() {return NULL;}
        
          void setReactor(IReactor *) {return;}

          void EventCallback() {return;}
        
          void setCallback(work_handle cb, void *p) {return;}

          timeval * timeout() {return NULL;}
        
          void setTimeout(int msec) {return;}

          int type()  {return 0;}
        
          void setType(int) { return;}

          int status() {return 0;}
        
          void setStatus(int) {return;}
        
          int result() {return 0;}
        
          void setResult(int) {return;}
        
          void derived() {return;}
        
          void setDerived(int) {return;}

          int addRef() {return 0;}

          int getRefCnt() {return 0;}

          bool release() {return false;}

          int delRef() {return 0;}
//===================================================//
          IEvent * next() {return nextNode;}
        
          void setNext(IEvent * node) {nextNode = node;}
        
          IEvent * previous() {return previousNode;}
        
          void setPrevious(IEvent * node) {previousNode = node;}
//==================================================//
          bool isError() {return true;}
          
          int label;
          ~TestEvent() {}

    private:
        IEvent * nextNode;
        IEvent * previousNode;

};

int main()
{
    EQueue *q = new EQueue();
    TestEvent * tmp;
    tmp = (TestEvent *)q->pop();
    printf("first empty : %d\n", 0);

    for(size_t i = 0; i < 4; i ++)
    {
        int j = 0;
        while(j < i)
        {
            tmp = new TestEvent();
            tmp->label = j;
            q->push(tmp);
            j++;
        }
        j = 0;
        while(j < i + 2)
        {
            tmp = (TestEvent *)q->pop();
            if(tmp)
                printf("pop num : %d value %d\n", 1, tmp->label);
            else
                printf("pop num : %d empty\n", 0);
            printf("used %d \n", q->size());
            j++;
        }
    }
    printf("=======================================\n");
    IEvent * test2_push[10];
    IEvent * test2_pop[10];
    for(size_t i = 0; i < 10; i++)
    {
        test2_push[i] = new TestEvent(i+1);
    }
    int cnt;
    cnt = q->pushs(&test2_push[0], 5);
    printf("push %d\n", cnt);
    cnt = q->pushs(&test2_push[5], 5);
    printf("push %d\n", cnt);

   //=============================================
   
    cnt = q->pops(&test2_pop[0], 3);
    printf("pop %d\n", cnt);
    
    for(size_t i = 0; i < cnt; i++)
        printf("value : %d\n", ((TestEvent *)(test2_pop[i]))->label);
    
    cnt = q->pops(&test2_pop[3], 7);
    printf("pop %d\n", cnt);

    for(size_t i = 0; i < cnt; i++)
        printf("value : %d\n", ((TestEvent *)(test2_pop[i + 3]))->label);
   
    tmp = (TestEvent *)q->pop();
    if(NULL == tmp)
       printf("pop 0\n");

    cnt = q->pops(&test2_pop[0], 1);
    printf("pop %d\n", cnt); 
    return 0;
}

