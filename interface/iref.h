#ifndef _IREF_H_
#define _IREF_H_

class IRef
{
    public:
        
        virtual int addRef() = 0;
        
        virtual int delRef() = 0;
        
        virtual int getRefCnt() = 0;
        
        virtual bool release() = 0;
        
        IRef(): cnt(1)
        {}
        
        virtual ~IRef(){}  

     protected:
        int cnt;
};


template <class T>
class SmartPtr
{
    public:
        SmartPtr() : event(0)
        {}
        
        explicit SmartPtr(T * t) : event(t) 
        {}
        
        SmartPtr(const SmartPtr & rhs)
        {
            rhs.event->addRef();
            event = rhs.event;
        }

        ~SmartPtr() 
        {
            if (event) 
            {
                event->release();
            }
        }

        void destroyPtr() 
        {
            if (event) 
            {
                delete event;
                event = 0;
            }
        }


        inline SmartPtr & operator = (SmartPtr &rhs)
        {
            if (event == rhs.event) 
            { 
                return *this; 
            }
            if (event) 
            {
                if(event->release())
                    event = 0;
            }
            rhs.event->addRef();
            event = rhs.event;
            return *this;
        }

        inline T & operator * () { return event; }
        inline T * operator -> () { return event; }
        inline T * operator & () { return event; }

    private:
        T * event;
};

#endif
