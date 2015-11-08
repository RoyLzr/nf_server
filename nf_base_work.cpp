#include "nf_base_work.h"

static const char tstLabel = '!';

int NonBlockReadLine :: work(int fd, 
                             void * arg)
{
    
    ReadEvent * ev = (ReadEvent *) arg;
    CWorkerThread *cth = get_pthread_data();
    char * tmp;
    int tmp_len;

    if(cth == NULL) 
    {
        tmp = (char *) malloc(sizeof(1024));
        tmp_len = 1024;
    }
    else
    {
        tmp = (char *) cth->GetWorkerBuff();
        tmp_len = cth->GetWorkerLen();
    }

    Reactor * rect = ev->get_reactor();
    struct evepoll * eve = rect->get_fds(fd);
    WriteEvent * w_ev = (WriteEvent *) eve->evwrite;
    Buffer & buff = ev->get_buffer();
    int n;
    char * show;

    if(set_fd_noblock(fd) < 0)
        return -1;

    if ((n = readn(fd, tmp, tmp_len)) <= 0)
    {
        Log :: WARN("Read Error FD : %d Error %s code: %d", \
                    fd, strerror(errno), n);
        return -1;
    }
    tmp[n] = '\0';
    if(cth != NULL)
    {
        Log :: DEBUG("handle Thread %d, [Read Event] len : %d, Data : %s", \
                     cth->GetThreadIdx(), n, tmp);
    }

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
        
        if(w_ev->get_buf_unhandle_num() > 0)
        {
            NonBlockFun * w_handle = w_ev->get_parse_handle();
            w_handle->work(fd, w_ev);
        }
        
        if(w_ev->get_buf_unhandle_num() <= 0)
        {
            ev->add_ev_flags(EV_READUNFIN);
        }
    }
}

int NonBlockWrite :: work(int fd, void * arg) 
{
    WriteEvent * w_ev = (WriteEvent *) arg;
    int num = w_ev->get_buf_unhandle_num();
    Buffer & w_buff = w_ev->get_buffer();
    void * src = w_buff.get_unhandle_cache();
    
    ev_handle callback = w_ev->get_ev_handle();
    callback(0, EV_WRITE, NULL);
    int len = w_buff.get_unhandle_num(); 
    int n = 0;
    
    if (n = sendn(fd, (char *)src, w_buff.get_unhandle_num()) <=0 )
    {
        Log :: WARN("Write Error FD : %d Error %s code: %d", \
                    fd, strerror(errno), n);
        return -1;
    }

#ifndef WORK
    if(len > 0)
    {
        *((char *)src + len) = '\0';
        Log :: DEBUG("OUtPut write len %d data %s", \
                      n, (char *)src);
    }
#endif

    w_buff.add_handl_num(w_buff.get_unhandle_num());

    if(w_ev->get_buf_unhandle_num() > 0)
    {
        w_ev->add_ev_flags(EV_WRITEUNFIN);
    }
}


NonBlockReadLine * parseLine()
{
    static NonBlockReadLine * r_fun = new NonBlockReadLine();
    return r_fun;
}


NonBlockWrite * writeData()
{
    static NonBlockWrite * w_fun = new NonBlockWrite();
    return w_fun;
}


