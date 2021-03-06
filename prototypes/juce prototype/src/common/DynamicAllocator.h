/******************************************************************************
          .d8888b.   .d8888b.  
         d88P  Y88b d88P  Y88b 
         888    888        888 
88888b.  888    888      .d88P 
888 "88b 888    888  .od888P"  
888  888 888    888 d88P"      
888  888 Y88b  d88P 888"       
888  888  "Y8888P"  888888888              Open Kaillera Arcade Netplay Library
*******************************************************************************
Copyright (c) Open Kaillera Team 2003-2008

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice must be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/
#pragma once

#include "DynamicArray.h"
#include "ListGenerics.h"

namespace n02 {

    template <class _BaseType, int _BlockLen=32, int _MaximumAllocation=999>
    class DynamicAllocator
    {

    protected:

        /* list of allocated items */
        DynamicArray<_BaseType*, _BlockLen> allocated;

    public:

        /* distructor */
        ~DynamicAllocator()
        {
            resetAllocation();
        }

        /* allocate a single object */
        inline _BaseType * allocate()
        {
            register _BaseType * return_value = new _BaseType;
            allocated.addItem(return_value);
            return return_value;
        }

        /* free an allocated item */
        inline void free(_BaseType * element)
        {
            require(allocated.itemsCount() > 0);
            require(DynamicArray_find<_BaseType *>((DynamicArray<_BaseType *>&)allocated, element) != -1);
            allocated.removeItem(element);
            delete element;
        }

        /* returns the number of allocated object */
        inline int allocatedCount() const
        {
            return allocated.itemsCount();
        }

        /* returns the number of free objects */
        inline int freeCount() const
        {
            return _MaximumAllocation - allocated.itemsCount();
        }

        /* reset all allocation */
        inline void resetAllocation()
        {
            if (allocated.itemsCount()) {
                for (int x = 0; x < allocated.itemsCount(); x++) {
                    delete allocated[x];
                }
            }
            allocated.clearItems();
        }
    };

};
