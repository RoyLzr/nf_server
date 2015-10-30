#include "nf_base_work.h"

static const char tstLabel = '!';

int parseLine(int fd, void * arg)
{
    ReadEvent * ev = (ReadEvent *) arg;
    CWorkerThread *cth = get_pthread_data(); 
    char * tmp = (char *) cth->GetWorkerBuff();
    int tmp_len = cth->GetWorkerLen();
    Reactor * rect = ev->get_reactor();
    struct evepoll * eve = rect->get_fds(fd);
    WriteEvent * w_ev = (WriteEvent *) eve->evwrite;
    Buffer & buff = ev->get_buffer();
    int n;
    char * show;

    if(set_fd_noblock(fd) < 0)
        return -1;

    ev->del_ev_flags(EV_READUNFIN);

    if ((n = readn(fd, tmp, tmp_len)) < 0 )
    {
        Log :: WARN("Read Error FD : %d Error %s", \
                    fd, strerror(errno));
    }
    tmp[n] = '\0';
    Log :: DEBUG("handle Thread %d, [Read Event] len : %d, Data : %s", \
                 cth->GetThreadIdx(), n, tmp);

    ev_handle call_back = ev->get_ev_handle();

    if(call_back != NULL)
    {
        for(int i = 0; *(tmp + i) != '\0'; i++)
        {
            if(*(tmp + i) == tstLabel)
            {
            #ifndef WORK
                printf("SEARCHING REQ LINE POS : %d VAL : %s", \
                        i, tmp);
            #endif
                buff.add_data(tmp, i + 1);
                call_back(fd, EV_READ, eve);
                tmp += (i + 1);
                n -= (i + 1);
                i = -1;  
            }
        }

        if(n > 0)
        {
            buff.add_data(tmp, n);
            Log :: DEBUG("READ DUMP CACHE %d bytes VAL : %s ", \
                         n, buff.get_unhandle_cache());
        #ifndef WORK
            printf("READ DUMP CACHE %d bytes VAL : %s ", \
                   n, buff.get_unhandle_cache());
        #endif
        }
        parse_handle p_handle = w_ev->get_parse_handle();
        p_handle(fd, w_ev);
        
        if(w_ev->get_buf_unhandle_num() <= 0)
        {
            ev->add_ev_flags(EV_READUNFIN);
        }
    }
}

int sendData(int fd, void * arg)
{
    WriteEvent * w_ev = (WriteEvent *) arg;
    int num = w_ev->get_buf_unhandle_num();
    Buffer & w_buff = w_ev->get_buffer();
    void * src = w_buff.get_unhandle_cache();

    w_ev->del_ev_flags(EV_WRITEUNFIN);
     
    int n = write(fd, (char *)src, w_buff.get_unhandle_num());
    w_buff.add_handl_num(w_buff.get_unhandle_num());

#ifndef WORK
    Log :: DEBUG("OUtPut write len %d data %s", \
                  n, (char *)src);
#endif

    if(w_ev->get_buf_unhandle_num() > 0)
    {
        w_ev->add_ev_flags(EV_WRITEUNFIN);
    }
    
}

