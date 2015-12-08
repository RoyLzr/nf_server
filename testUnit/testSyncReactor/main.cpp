#include "../../NfUnit/eventqueue.h"
#include "../../NfUnit/extreactor.h"
#include "../../interface/ievent.h"
#include "../../commonn/ThreadManager.h"
#include "../../NfUnit/syncReactor.h"
#include "../../NfUnit/baseEvent.h"
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>

class TestEvent : public EventBase
{
    public:
          TestEvent() {};
          TestEvent(int num) : label(num)  {};
          ~TestEvent() {printf("clear\n");}
          void EventCallback()
          {
              printf("ev : %d is ext \n", label);
              this->setStatus(IEvent::DONE);
          }
    private:
          int label;
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
    ev->setReUsed(false);
  
    net->post(ev);   
    net->run();

    return 0;    
}

