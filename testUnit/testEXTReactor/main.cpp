#include "../../NfUnit/eventqueue.h"
#include "../../NfUnit/extreactor.h"
#include "../../interface/ievent.h"
#include "../../commonn/ThreadManager.h"
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

class TestEvent : public IEvent
{
    public:
          TestEvent() {};
          TestEvent(int num) : label(num)  {};
          int handle() { return 0;}
        
          void setHandle(int) {return;}

          IReactor *reactor() {return NULL;}
        
          void setReactor(IReactor *) {return;}

          void EventCallback() {printf("ev : %d is ext \n", label);}
        
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
        
          int addRef()
          {
              cnt++;
              return cnt;
          }
          int delRef()
          {
              cnt--;
              return cnt;
          }
          int getRefCnt()
          {return cnt;}
          bool release()
          {
              if(delRef() <= 0)
              {
                  printf("clear \n");
                  delete this;
                  return true;
              }
              return false;
          }
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
    IEQueue * _queue = new BlockEQueue();
    EXTReactor * ext = new EXTReactor();
    ext->setQueue(_queue);
    ext->run();
    int i = 0;
    sleep(1);
    while(true)
    {
        IEvent * ev = new TestEvent(i);
        i++;
        SmartPtr<IEvent> sev(ev);
        //Test event reuse, set ev->addRef()
        //ev->addRef();
        ext->post(&sev);    
    } 

    return 0;    
}

