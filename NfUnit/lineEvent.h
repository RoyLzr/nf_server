#ifndef  _LINEEVENT_H_
#define  _LIENEVENT_H_

#include "baseEvent.h"
#include "../Buffer.h"

class LineEvent : public SockEventBase 
{
    public:
        typedef void (* LINE_READ_CALLBACK_T)(LineEvent *);
        typedef void (* LINE_WRITE_CALLBACK_T)(LineEvent *);

        LineEvent() : _read_done_callback(0),
                      _write_done_callback(0),
                      _read_buff_flag(-1),
                      _write_buff_flag(-1)
        {}   


        ~LineEvent() {};

        void initBuffer(int); 
    
        inline Buffer & get_read_buff(){ return _readBuffer; }
        
        inline Buffer & get_write_buff() { return _writeBuffer; }

        inline void set_read_done_callback(LINE_READ_CALLBACK_T read_cb)
        {
            _read_done_callback = read_cb;
        }
        
        inline void set_write_done_callback(LINE_WRITE_CALLBACK_T write_cb)
        {
            _write_done_callback = write_cb;
        }
        
        inline int get_read_buff_flag()
        {
            return _read_buff_flag;
        }       
    protected:

        virtual void read_callback();
        virtual void accept_callback() {return;}
        virtual void write_callback();
        virtual void tcpconnect_callback() {return;}
        virtual void read_done_callback();
        virtual void write_done_callback();

    protected:
        
        LINE_READ_CALLBACK_T   _read_done_callback;
        LINE_WRITE_CALLBACK_T  _write_done_callback;

        Buffer       _readBuffer;
        Buffer       _writeBuffer;
        int          _read_buff_flag;
        int          _write_buff_flag;
};

#endif
