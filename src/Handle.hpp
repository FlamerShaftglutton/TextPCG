#pragma once

namespace ECS
{
    //was going to use a specific int length (like int64_t)
    //but int is word-sized for the processor, so slightly faster
    //and even on a 32 bit machine this'll provide over 2 billion 
    //unique handles for each type before recycling
    typedef int Handle;
    
    /*
    class Handle
    {
        int h;
    public:
        Handle() : h(0) { }
        Handle(int H) : h(H) { }
        Handle& operator=(int H)
        {
            h = H;
            
            return *this;
        }
        
        Handle& operator=(const Handle& rhs)
        {
            h = rhs.h;
            
            return *this;
        }
    };
    */
}