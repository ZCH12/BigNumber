//
//  BigNumber.h
//  BigNumber
//
//  Created by 张驰荣 on 2016/12/29.
//  Copyright © 2016年 张驰荣. All rights reserved.
//

#ifndef BigNumber_h
#define BigNumber_h

#include <iostream>
#include <string>

#define DEFAULT_ACCURACY 8      //默认小数的精度保留8位,如果有更高精度的需求,可修改它

//定义超出长度的数字的处理方式
//1.截断
//2.抛出异常
//3.改变对象（重新分配）

class BFException
{
private:
    std::string message;
public:
    BFException(std::string detail);
    ~BFException();
    void GetMessage();       //输出错误信息
};


class BigFigure
{
private:
    struct BFDetail
    {
        size_t ReferCount;      //引用计数,用于判断何时销毁
        size_t AllocatedSize;   //数字字符串分配的大小
        size_t Accuracy;        //浮点数部分的最大精确度
        bool Minus;             //表示是否为负数,如果为负数,则该值为1
        char *NumInt;           //可输出的整数部分的字符串的首地址
        char *NumFloat;         //可输出的浮点数部分的字符串的首地址
        char *StringHead;       //保存申请的字符串空间的首指针
    } *Detail;
    void FreeDetail();
public:
    BigFigure();
    BigFigure(size_t size);
    BigFigure(const BigFigure& Base);
    ~BigFigure();
    void core_add();
    
};



#endif
