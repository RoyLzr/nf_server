#include "../../NfUnit/eventqueue.h"
#include "../../NfUnit/extreactor.h"
#include "../../interface/ievent.h"
#include "../../commonn/ThreadManager.h"
#include "../../commonn/singleton.h"
#include "../../NfUnit/syncReactor.h"
#include "../../NfUnit/baseEvent.h"
#include "../../NfUnit/lineEvent.h"
#include "../../net.h"
#include "../../nf_server.h"
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
    Singleton<ConfigParser>::instance()->parser_file("./svr.conf");
    Singleton<ConfigParser>::instance()->scan();
    const Section & actSec = Singleton<ConfigParser>::instance()->get("NETREACTOR");
    const Section & svrSec = Singleton<ConfigParser>::instance()->get("SERVER");

    IEQueue *     queue = new BlockEQueue();
    EXTReactor  * ext = new  EXTReactor();
    ext->setQueue(queue);
    
    SyncReactor * net = new SyncReactor();
    net->setExtReactor(ext);
    net->load(actSec);

    LineEvent * ev = new LineEvent();
    ev->set_read_done_callback(user_read_done_callback);
    ev->set_write_done_callback(0);

    NfSvr mySvr(net);
    mySvr.init(svrSec);
    mySvr.set_reactor(net);
    mySvr.set_acc_event(ev);
    mySvr.run();

    return 0;    
}

