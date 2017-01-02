#include <memory>
#include <cstdio>
//默认的长度分配内存的大小

/*
 超长数字运算库
 待实现：加减乘除模，
*/

#include "BigNumber.h"

BFException::BFException(int ErrVal, std::string detail)
{
	message = detail;
}
BFException::~BFException()
{

}

BigFigure::BigFigure()
{

}

//以指定大小初始化一个BigFigure,初始化后的值为0
BigFigure::BigFigure(size_t IntSize, size_t FloatSize) throw(...)
{
	int AllocatedMem;
	if (IntSize == 0)
		throw BFException(ERR_ILLEGALPARAM, "整数部分分配的内存不能为0");
	AllocatedMem = IntSize + 1;
	if (FloatSize > 0)
		AllocatedMem += FloatSize + 1;

	try {
		Detail = new BFDetail;
		Detail->Accuracy = FloatSize;
		Detail->AllocatedSize = AllocatedMem;
		Detail->ReferCount = 1;
		Detail->IntLen = 1;						//初始化的长度为0
		Detail->Minus = 0;						//
		try {
			Detail->StringHead = new char[AllocatedMem]();
			Detail->StringHead[IntSize - 1] = '0';			//初始化数字为0
			Detail->NumInt = Detail->StringHead + IntSize - 1;

			if (FloatSize > 0) {
				Detail->NumFloat = Detail->StringHead + IntSize + 1;
				memset(Detail->NumFloat, 0, sizeof(char)*FloatSize);
			}
		}
		catch (const std::bad_alloc)
		{
			delete Detail;
			throw BFException(ERR_MEMORYALLOCATEDEXCEPTION, "数字字符串分配错误");
		}
	}
	catch (const std::bad_alloc)
	{
		throw BFException(ERR_MEMORYALLOCATEDEXCEPTION, "BFDetail分配错误");
	}
	catch (const BFException& e)
	{
		throw e;
	}
}

BigFigure::BigFigure(const BigFigure& Base)
{

}

BigFigure::~BigFigure()
{

}

/*****************************************************************************************
算术运算核心函数
******************************************************************************************/
void BigFigure::core_IntAdd(BigFigure & result, BigFigure & OperandA, BigFigure & OperandB)
{

}

/******************************************************************************************
重载函数
*******************************************************************************************/







/******************************************************************************************
基础函数
*******************************************************************************************/

/*
检查字符串是否为数字,并且判断字符串表示的是整数还是小数
返回值代表的含义如下:
  1.整数
  2.小数
  3.科学计数法
  0.非数字
*/
int NumCheck(std::string CheckString)
{
	size_t index_p;
	size_t len = CheckString.length();
	const char *tempString = CheckString.c_str();
	bool hasPoint = false;			//记录是否有小数点
	bool scinotation = false;		//记录是否以科学计数法进行标记
	bool minus1 = false;			//记录数字是否有负号
	bool minus2 = false;			//记录科学计数法时幂指数是否有负号
	bool hasNumPre = false;			//记录在指定符号前方是否有数字
	for (index_p = 0; index_p < len; index_p++)
	{
		if (tempString[index_p] >= '0'&&tempString[index_p] <= '9')
			hasNumPre = true;
		else if (tempString[index_p] == '.')
		{
			if (hasPoint)
			{
				//已经有小数点,又遇到一个小数点,此时是非法数字
				return 0;
			}
			else if (scinotation)
			{
				//在科学计数法的指数部分发现小数,此时的非法数字
				return 0;
			}
			else
				hasPoint = true;
		}
		else if (tempString[index_p] == 'E' || tempString[index_p] == 'e')
		{
			if (hasNumPre&&index_p != len - 1) {
				if (scinotation)
				{
					//在科学计数法的指数部分又出现一个指数,此时为非法数字
					return 0;
				}
				else {
					scinotation = true;
					hasNumPre = false;
				}
			}
			else
			{
				//如果遇到E在字符串结尾,是一个非法数字
				return 0;
			}
		}
		else if (tempString[index_p] == '-')
		{
			if (!hasNumPre&&index_p != len - 1) {
				if (scinotation)
				{
					//科学计数法状态时,已经读取到指数部分,遇到负号修改minus2
					if (minus2)//判断重复负号的情况
						return 0;
					else
						minus2 = true;
				}
				else {
					if (minus1)//判断重复负号的情况
						return 0;
					else
						minus1 = true;
				}
			}
			else
			{
				//如果遇到负号在字符串结尾,也是非法数字
				return 0;
			}
		}
		else {
			//发现其他字符,非法数字
			return 0;
		}
	}
	//如果是正确的数字,则可以通过这个for的验证,接下来判断数字的类型
	if (scinotation)
		return 3;
	else if (hasPoint)
		return 2;
	else
		return 1;
}




