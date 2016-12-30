//
//  BigNumber.cpp
//  BigNumber
//
//  Created by 张驰荣 on 2016/12/29.
//  Copyright © 2016年 张驰荣. All rights reserved.
//

//默认的长度分配内存的大小

/*
 超长数字运算库
 待实现：加减乘除模，
*/

#include "BigNumber.h"

BFException::BFException(std::string detail)
{
    message=detail;
}
BFException::~BFException()
{
    
}

BigFigure::BigFigure()
{
    
}

//以指定大小初始化一个BigFigure
BigFigure::BigFigure(size_t size)
{
    if (size<=DEFAULT_ACCURACY+2)
        throw BFException("size分配的内存太小");
    
    Detail=new BFDetail();
    Detail->Accuracy=DEFAULT_ACCURACY;
    Detail->AllocatedSize=size;
    Detail->StringHead=new char[size]();
    
}

BigFigure::BigFigure(const BigFigure& Base)
{

}

BigFigure::~BigFigure()
{
    
}










