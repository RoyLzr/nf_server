#include "../../interface/iref.h"
#include "stdio.h"
#include "stdlib.h"

class Test : public IRef
{
    public:
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
                printf("clear \n");
                delete this;
                return true;
            }
            return false;
        }
};

void test(SmartPtr<Test> sptr)
{
    SmartPtr<Test> scopy(sptr);
    return;
}

int main()
{
    int i = 10;
    while(i)
    {
        Test * tt = new Test();
        SmartPtr<Test> stt(tt);
        i--;
    }
    printf("==============================\n");
    printf("testFun\n");
    
    { 
        Test * tt1 = new Test();
        SmartPtr<Test> stt1(tt1);
        test(stt1);
    }

    printf("==============================\n");
    
    Test * tt = new Test();

    SmartPtr<Test> stt(tt); 
    SmartPtr<Test> sequal;
    SmartPtr<Test> scopy(stt);
    
    sequal = stt;
    
    
    return 0;
}

