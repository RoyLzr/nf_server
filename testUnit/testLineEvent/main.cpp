#include "../../NfUnit/eventqueue.h"
#include "../../NfUnit/extreactor.h"
#include "../../interface/ievent.h"
#include "../../commonn/ThreadManager.h"
#include "../../NfUnit/syncReactor.h"
#include "../../NfUnit/baseEvent.h"
#include "../../NfUnit/lineEvent.h"
#include "../../net.h"
#include "stdio.h"
#include "stdlib.h"
#include <unistd.h>


void user_read_done_callback(LineEvent * ev)
{
    Buffer & rBuff = ev->get_read_buff();
    Buffer & wBuff = ev->get_write_buff();
   
    wBuff.add_data(rBuff.get_unhandle_cache(), ev->get_read_buff_flag()); 
    
    rBuff.add_handl_num(ev->get_read_buff_flag());
    
    return;
}

int main()
{
    Log :: init("./svr.log");
    Log :: set_level(0);

    IEQueue * queue = new BlockEQueue();
    SyncReactor * net = new SyncReactor();
    EXTReactor  * ext = new  EXTReactor();
    ext->setQueue(queue);
    net->setExtReactor(ext);
    
    LineEvent * ev = new LineEvent();
    
    if(set_fd_noblock(1) < 0)
        printf("set fd noblock error\n");

    ev->registerRead(1, 1024);
    ev->setReUsed(false);
    ev->set_read_done_callback(user_read_done_callback);

    net->post(ev);   
    net->run();

    return 0;    
}

