#include "lineEvent.h"

void LineEvent::initBuffer(int size)
{
    if(_readBuffer.isEmpty())
       _readBuffer.init(size);
    if(_writeBuffer.isEmpty())
        _writeBuffer.init(size);
}

void LineEvent::read_done_callback()
{
    if(NULL != _read_done_callback)
        _read_done_callback(this);
}

void LineEvent::write_done_callback()
{
    if(NULL != _write_done_callback)
        _write_done_callback(this);
}

void LineEvent::read_callback()
{
    int n = 0;
    int emptySize = _readBuffer.get_empty_size();
    char * tmp = NULL;
    int  size = 0;     

    if(emptySize < 512)
    {
        Log::WARN("LineEvent::read_callback \
                design Buffer size error, \
                not enough to read from net, emptySize : %d", emptySize);
        _readBuffer.fresh_cache(512);    
    }

#ifndef WORK
    printf("before read empty size %d allo_len %d\n", 
            _readBuffer.get_empty_size(), 
            _readBuffer.get_allo_size());
#endif
    //read done, must add unhandle num;
    if((n = readn(_fd, 
                  _readBuffer.get_empty_cache(),
                  _readBuffer.get_empty_size())) < 0)
    {
        Log::WARN("LineEvent::read_callback error: %s  fd : %d", 
                strerror(errno), _fd);
        this->setStatus(IEvent::CANCELED);
        this->setResult(IEvent::ERROR);
        return ;
    }
    _readBuffer.add_unhandle_num(n);
    
    tmp  = static_cast<char *> (_readBuffer.get_unhandle_cache());
    size = _readBuffer.get_unhandle_num();
    
    for(int i=0; i<size; i++)
    {
        if(*(tmp+i) == '\n')
        {
            _read_buff_flag = i + 1;
            read_done_callback();            
        }
    }  
    //optimize empty space 
    _readBuffer.check_empty_space();
#ifndef WORK
    printf("after read empty size %d\n", 
            _readBuffer.get_empty_size());
#endif
    
    //optimize write, reduce reactor write loop
    if(_writeBuffer.get_unhandle_num()>0)
        write_callback();
    if(_writeBuffer.get_unhandle_num() > 0)
        this->registerWrite(_fd,
                 _writeBuffer.get_unhandle_num());
    return;
}

void LineEvent::write_callback()
{
    int n = 0;
    int len = _writeBuffer.get_unhandle_num();
    if((n = sendn(_fd, 
                  _writeBuffer.get_unhandle_cache(),
                  len)) <=0)
    {
        Log::WARN("LineEvent::write_callback error: %s  fd : %d", 
                  strerror(errno), _fd);
        this->setStatus(IEvent::CANCELED);
        this->setResult(IEvent::ERROR);
        return ;
    }
    _writeBuffer.add_handl_num(n);

    if(_writeBuffer.get_unhandle_num() <= 0) 
        this->registerRead(_fd, 0);

    return ;
}


