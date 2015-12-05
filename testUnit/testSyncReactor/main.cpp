#include "../../NfUnit/eventqueue.h"
#include "../../NfUnit/extreactor.h"
#include "../../interface/ievent.h"
#include "../../commonn/ThreadManager.h"
#include "../../NfUnit/syncReactor.h"
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

class TestEvent : public IEvent
{
    public:
          TestEvent() {};
          TestEvent(int num) : label(num)  {};
          int handle() { return _fd;}
        
          void setHandle(int hand) { _fd = hand;}

          IReactor *reactor() {return _rect;}
        
          void setReactor(IReactor * rect) { _rect = rect;}

          void EventCallback() 
          {
              printf("ev : %d is ext \n", label);
          }
        
          void setCallback(work_handle cb, void *p) {return;}

          timeval * timeout() {return NULL;}
        
          void setTimeout(int msec) {return;}

          int type()  {return _type;}
        
          void setType(int type) {_type = type;}

          int status() {return _status;}
        
          void setStatus(int status) {_status = status;}
        
          int result() {return _result;}
        
          void setResult(int res) {_result = res;}
        
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
            
          bool isReUsed() {return false;}         
          int label;
          ~TestEvent() {printf("clear\n");}

    private:
        IEvent * nextNode;
        IEvent * previousNode;
        IReactor * _rect;
        int _type;
        int _status;
        int _result;
        int _fd;
};


int main()
{
    Log :: init("./svr.log");
    Log :: set_level(0);
    IEQueue * queue = new BlockEQueue();
    SyncReactor * net = new SyncReactor();
    EXTReactor  * ext = new  EXTReactor();
    ext->setQueue(queue);
    net->setExtReactor(ext);

    IEvent * ev = new TestEvent(0);
    ev->setResult(IEvent::IOREADABLE);
    ev->setType(IEvent::NET);
    ev->setStatus(IEvent::INIT);
    ev->setHandle(1);

    net->post(ev);   
    net->run();

    return 0;    
}

