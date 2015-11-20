#include "nf_base_work.h"

static const char tstLabel = '\n';

int NonBlockReadLine :: work(int fd, 
                             void * arg)
{
    
    ReadEvent * ev = (ReadEvent *) arg;
    CWorkerThread *cth = get_pthread_data();
    char data[1024];
    char * tmp = NULL;
    int tmp_len = 0;
    int ret = 0;


    if(cth != NULL)
    {
        tmp = (char *) cth->GetWorkerBuff();
        tmp_len = cth->GetWorkerLen();
    }
    else
    {
        tmp = data;
        tmp_len = 1024;
    }

    Reactor * rect = ev->get_reactor();
    struct evepoll * eve = rect->get_fds(fd);
    WriteEvent * w_ev = (WriteEvent *) eve->evwrite;
    Buffer & buff = ev->get_buffer();
    int n;
    char * show;

    if(set_fd_noblock(fd) < 0)
        return -1;

    if ((n = readn(fd, tmp, tmp_len)) < 0)
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
                
                if((ret = call_back(fd, EV_READ, eve)) < 0)
                    return ret;
                
                tmp += (i + 1);
                n -= (i + 1);
                i = -1;  
            }
        }
      #ifndef WORK 
        if(n == 1024)
        {
            printf("1024 NO data client Error\n");
        }
     #endif

        if(n > 0)
        {
            buff.add_data(tmp, n);
            Log :: DEBUG("READ DUMP CACHE %d bytes", \
                         n);
        #ifndef WORK
            printf("READ DUMP CACHE %d bytes", \
                   n);
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
    int ret = 0;

    ev_handle callback = w_ev->get_ev_handle();
    if((ret = callback(0, EV_WRITE, NULL)) < 0)
        return ret;

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
        Log :: DEBUG("OUtPut write len %d", \
                      n);
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


