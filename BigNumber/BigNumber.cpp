#define _CRT_SECURE_NO_WARNINGS
#include <memory>
#include <cstdio>
#include <cstdlib>
//默认的长度分配内存的大小

/*
 超长数字运算库
 待实现：加减乘除模，
*/

#include "BigNumber.h"

//常量定义
#define CONST_OVER9 106		//大于等于这个数的为两个数相加大于'9'的
//#define CONST_BELOW0 95		//小于等于这个数的为两个数相减


//控制显示方式的变量
bool ReserveZero = true;			//保留小数后的0
bool ScinotationShow = false;		//不以科学计数法显示数字
bool ConfirmWontLossAccuracy = true;//确保不截断小数的有效位(关闭之后如果赋值时小数位太多,则会省略多出的部分;如果开启,则会抛出异常)
bool ConfirmWontLossHighBit = true;	//确保不丢失整数的高位(如果发生溢出时),如果为true,则发生溢出时会抛出异常,如果为false,则如果溢出则舍弃高位
size_t ScinotationLen = 5;			//科学计数法时有效位数为5位



/*****************************************************************************************
构造函数
******************************************************************************************/
BFException::BFException(int ErrVal, std::string detail)
{
	message = detail;
}
BFException::~BFException()
{

}


NumStringDetail::NumStringDetail(std::string NumString) :
	RadixMinus(0), ExpMinus(0), IntBeZero(0), Mode(0), IntLen(0), FloatLen(0), ExpLen(0), IntStart_p(0), FloatStart_p(0), ExpStart_p(0)
{
	this->NumString = NumString;
	NumCheck(*this);
}
NumStringDetail::NumStringDetail(double Num) :
	RadixMinus(0), ExpMinus(0), IntBeZero(0), Mode(0), IntLen(0), FloatLen(0), ExpLen(0), IntStart_p(0), FloatStart_p(0), ExpStart_p(0)
{
	char TempString[32];
	_gcvt(Num, 16, TempString);
	this->NumString = std::string(TempString);
	NumCheck(*this);
}
NumStringDetail::NumStringDetail(__int64 Num) :
	RadixMinus(0), ExpMinus(0), IntBeZero(0), Mode(0), IntLen(0), FloatLen(0), ExpLen(0), IntStart_p(0), FloatStart_p(0), ExpStart_p(0)
{
	char TempString[32];
	sprintf(TempString, "%I64d", Num);
	this->NumString = std::string(TempString);
	NumCheck(*this);
}
NumStringDetail::NumStringDetail(long Num) :
	RadixMinus(0), ExpMinus(0), IntBeZero(0), Mode(0), IntLen(0), FloatLen(0), ExpLen(0), IntStart_p(0), FloatStart_p(0), ExpStart_p(0)
{
	char TempString[32];
	sprintf(TempString, "%ld", Num);
	this->NumString = std::string(TempString);
	NumCheck(*this);
}
NumStringDetail::NumStringDetail(int Num) :
	RadixMinus(0), ExpMinus(0), IntBeZero(0), Mode(0), IntLen(0), FloatLen(0), ExpLen(0), IntStart_p(0), FloatStart_p(0), ExpStart_p(0)
{
	char TempString[16];
	sprintf(TempString, "%d", Num);
	this->NumString = std::string(TempString);
	NumCheck(*this);
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
		Detail->IntAllocatedLen = IntSize;
		Detail->ReferCount = 1;
		Detail->IntLen = 1;						//初始化的长度为0
		Detail->Minus = false;					//为无负号
		Detail->Illage = false;					//标记为合法数字
		try {
			Detail->StringHead = new char[AllocatedMem]();
			Detail->IntTail = Detail->StringHead + IntSize;
			Detail->NumInt = Detail->IntTail - 1;
			Detail->NumInt[0] = '0';	//初始化数字为0

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

//复制构造函数
BigFigure::BigFigure(const BigFigure& Base)
{
	this->Detail = Base.Detail;
	this->Detail->ReferCount++;
}

BigFigure::~BigFigure()
{
	Detail->ReferCount--;
	if (!Detail->ReferCount)
	{
		//当引用计数为0时才释放Detail所占的内存
		delete[] Detail->StringHead;
		delete Detail;
	}
	return;
}

/*****************************************************************************************
算术运算核心函数
******************************************************************************************/

//整数部分加法核心(两正整数相加)
/*
在调用前记得确保数字时有效的
*/
void core_IntAdd(BigFigure & result, const BigFigure & OperandA, const BigFigure & OperandB,int carry)
{
	//判断内存是否足够
	int buffer;							//计算时的缓冲区
	int index_r = result.Detail->IntAllocatedLen - 1, index_A = OperandA.Detail->IntLen - 1, index_B = OperandB.Detail->IntLen - 1;//两个对象正在处理的位的下标
	char *String1 = OperandA.Detail->NumInt, *String2 = OperandB.Detail->NumInt;
	//int carry = 0;

	if (index_r < index_A)
	{
		//内存不足以存放,准备报错
		if (ConfirmWontLossHighBit)
			throw BFException(ERR_NUMBERTOOBIG, "操作数A的值太大,无法存储到结果存储的对象中");
		else
		{
			//进行截断
			String1 += OperandA.Detail->IntLen - result.Detail->IntAllocatedLen;
			index_A = index_r;
		}
	}
	if (index_r < index_B)
	{
		//内存不足以存放,准备报错
		if (ConfirmWontLossHighBit)
			throw BFException(ERR_NUMBERTOOBIG, "操作数B的值太大,无法存储到结果存储的对象中");
		else
		{
			//进行截断
			String2 += OperandB.Detail->IntLen - result.Detail->IntAllocatedLen;
			index_B = index_r;
		}
	}

	while (index_A >= 0 && index_B >= 0)
	{
		buffer = String1[index_A] + String2[index_B] + carry;
		if (buffer >= CONST_OVER9)
		{
			//大于9,将要进行进位
			buffer -= 58;//58='9'+1
			carry = 1;
		}
		else
		{
			//数字没有大于9,不需要进位
			buffer -= '0';
			carry = 0;
		}
		result.Detail->StringHead[index_r--] = (char)buffer;
		index_A--;
		index_B--;
	}

	while (carry&&index_A >= 0)
	{
		buffer = String1[index_A] + carry;
		if (buffer >= '9')
		{
			//大于9,将要进行进位
			buffer -= 10;
			carry = 1;
		}
		else
		{
			//数字没有大于9,不需要进位
			carry = 0;
		}
		result.Detail->StringHead[index_r--] = (char)buffer;
		index_A--;
	}
	while (carry&&index_B >= 0)
	{
		buffer = String2[index_B] + carry;
		if (buffer >= '9')
		{
			//大于9,将要进行进位
			buffer -= 10;
			carry = 1;
		}
		else
		{
			//数字没有大于9,不需要进位
			carry = 0;
		}
		result.Detail->StringHead[index_r--] = (char)buffer;
		index_B--;
	}

	while (index_A >= 0)
		result.Detail->StringHead[index_r--] = String1[index_A--];
	while (index_B >= 0)
		result.Detail->StringHead[index_r--] = String2[index_B--];
	if (carry)
	{
		if (index_r >= 0)
		{
			result.Detail->StringHead[index_r--] = '1';
		}
	}
	result.Detail->Illage = false;
	result.Detail->IntLen = result.Detail->IntAllocatedLen - index_r - 1;
	result.Detail->NumInt = result.Detail->StringHead + index_r + 1;
	if (!ConfirmWontLossHighBit)
	{
		while (*result.Detail->NumInt == '0')result.Detail->NumInt++;
		result.Detail->IntLen = result.Detail->IntTail - result.Detail->NumInt;
		if (result.Detail->IntLen == 0)
		{
			result.Detail->IntLen = 1;
			result.Detail->NumInt = result.Detail->IntTail - 1;
			result.Detail->NumInt[0] = '0';
		}
	}



}
template <class T>
void core_IntAdd_Basis(BigFigure & result, const BigFigure & OperandA, T OperandB)
{
	int index_p = result.Detail->IntAllocatedLen - 1, index_r = OperandA.Detail->IntLen - 1;
	char *SourceString = OperandA.Detail->NumInt, *String3 = result.Detail->StringHead;
	int buffer, carry = 0;
	for (; index_r >= 0 && OperandB != 0; index_r--)
	{
		buffer = SourceString[index_r];
		buffer += OperandB % 10 + carry;
		OperandB /= 10;
		if (buffer > '9')
		{
			buffer -= 10;
			carry = 1;
		}
		else
		{
			carry = 0;
		}
		String3[index_p--] = (char)buffer;
	}

	if (OperandB != 0)
	{
		if (ConfirmWontLossHighBit)
		{
			result.Detail->Illage = false;
			throw BFException(ERR_NUMBERTOOBIG, "数字太大无法存储");
		}
	}
	else {
		while (carry&&index_p >= 0 && index_r >= 0)
		{
			buffer = SourceString[index_r] + carry;
			if (buffer > '9')
			{
				buffer -= 10;
				carry = 1;
			}
			else
			{
				carry = 0;
			}
			String3[index_p] = (char)buffer;
			index_p--, index_r--;
		}
		if (carry)
		{
			String3[index_p--] = '1';
			carry = false;
		}

	}
	while (index_p >= 0 && index_r >= 0)
		String3[index_p--] = SourceString[index_r--];

	result.Detail->IntLen = result.Detail->IntAllocatedLen - index_p - 1;
	result.Detail->NumInt = String3 + (index_p == -1 ? 0 : index_p) + 1;
	result.Detail->Illage = false;
}
/*
void core_IntAdd(BigFigure & result, const BigFigure & OperandA, double OperandB)
{
	BigFigure temp(1050, 1050);
	core_IntAdd(result, OperandA, temp.toBF(NumStringDetail(OperandB)));
}
*/
void core_IntAdd(BigFigure & result, const BigFigure & OperandA, int OperandB)
{
	core_IntAdd_Basis<int>(result, OperandA, OperandB);
}
void core_IntAdd(BigFigure & result, const BigFigure & OperandA, long OperandB)
{
	core_IntAdd_Basis<long>(result, OperandA, OperandB);
}
void core_IntAdd(BigFigure & result, const BigFigure & OperandA, __int64 OperandB)
{
	core_IntAdd_Basis<__int64>(result, OperandA, OperandB);
}
void core_IntAdd(BigFigure & result, const BigFigure & OperandA, short OperandB)
{
	core_IntAdd_Basis<short>(result, OperandA, OperandB);
}

//小数部分加法核心
int core_FloatAdd(BigFigure & result, const BigFigure & OperandA, const BigFigure & OperandB)
{
	int index_A, index_B = strlen(OperandB.Detail->NumFloat);
	char *String1 = OperandA.Detail->NumFloat, *String2 = OperandB.Detail->NumFloat, *String3 = result.Detail->NumFloat;
	int buffer;
	int carry = 0;

	if (OperandA.Detail->Accuracy)
		index_A = strlen(OperandA.Detail->NumFloat);
	else index_A = 0;

	if (OperandB.Detail->Accuracy)
		index_B = strlen(OperandB.Detail->NumFloat);
	else index_B = 0;


	if (index_A < index_B)
	{
		//如果A的小数位比B的短
		String3[index_B--] = 0;
		if ((int)result.Detail->Accuracy >= index_A)
		{
			while (index_B >= index_A)
			{
				String3[index_B] = String2[index_B];
				index_B--;
			}
		}
		else
		{
			if (ConfirmWontLossAccuracy)
			{
				//报错
				result.Detail->Illage = true;
				throw BFException(ERR_MAYACCURACYLOSS, "目标对象太小,无法存储足够的小数位,可能丢失精度");
			}
			else {
				//截断小数位,继续运行
				index_B = result.Detail->Accuracy;
				while (index_B >= index_A)
				{
					String3[index_B] = String2[index_B];
					index_B--;
				}
			}
		}
		index_A = index_B;							//将A,B同步,为下面只使用index_A做准备
	}
	else if (index_A > index_B)
	{
		//如果A的小数位比B的长
		String3[index_A--] = 0;
		if ((int)result.Detail->Accuracy >= index_B)
		{
			while (index_A >= index_B)
			{
				String3[index_A] = String1[index_A];
				index_A--;
			}
		}
		else
		{
			if (ConfirmWontLossAccuracy)
			{
				//报错
				result.Detail->Illage = true;
				throw BFException(ERR_MAYACCURACYLOSS, "目标对象太小,无法存储足够的小数位,可能丢失精度");
			}
			else {
				//截断小数位,继续运行
				index_A = result.Detail->Accuracy;
				while (index_A <= index_B)
				{
					String3[index_A] = String1[index_A];
					index_A--;
				}
			}
		}
	}
	else
	{
		String3[index_A--] = 0;
	}
	//从这里开始,两个字符串共用一个游标
	for (; index_A >= 0; index_A--)
	{
		buffer = String1[index_A] + String2[index_A] + carry;
		if (buffer >= CONST_OVER9)
		{
			buffer -= 58;
			carry = 1;
		}
		else
		{
			buffer -= '0';
			carry = 0;
		}
		String3[index_A] = (char)buffer;
	}

	return carry;
}



//实数加法运算
/*
未完成
*/
void IntAdd(BigFigure & result, BigFigure & OperandA, BigFigure & OperandB)
{
	bool minusA = OperandA.Detail->Minus, minusB = OperandB.Detail->Minus;

	if (!(minusA || minusB))
	{
		//正正相加
		core_IntAdd(result, OperandA, OperandB, core_FloatAdd(result, OperandA, OperandB));
		result.Detail->Minus = false;
	}
	else if (minusA && !minusB)
	{
		//负正相加
	}
	else if (!minusA&&minusB)
	{
		//正负相加
	}
	else {
		//负负相加
		core_IntAdd(result, OperandA, OperandB, core_FloatAdd(result, OperandA, OperandB));
		result.Detail->Minus = true;
	}
}
/*
等待
*/
void IntAdd(BigFigure & result, BigFigure & OperandA, double OperandB)
{
	BigFigure temp(1050, 1050);
	temp.toBF(NumStringDetail(OperandB));
	IntAdd(result,OperandA, temp);
}




//打印该对象的详细信息
void BigFigure::printDetail()
{
	printf(
		"字符串分配内存  :%d(%d+%d)\n"
		"整数部分有效位数:%d\n"
		"小数最大存储精度:%d\n"
		"以科学计数法输出:%s\n"
		"保留多余的0     :%s\n"
		"输出时有效位数  :%d\n"
		"存储的值        :%s\n",
		Detail->AllocatedSize, Detail->IntAllocatedLen + 1, Detail->AllocatedSize - Detail->IntAllocatedLen - 1,
		Detail->IntLen,
		Detail->Accuracy,
		ScinotationShow ? "是" : "否",
		ReserveZero ? "是" : "否",
		ScinotationLen,
		toString().c_str()
	);
	return;
}


/******************************************************************************************
重载函数
*******************************************************************************************/
BigFigure& BigFigure::operator=(const BigFigure &Source)
{
	return CopyDetail(Source);
}
BigFigure& BigFigure::operator=(const char* Source)
{
	this->toBF(NumStringDetail(std::string(Source)));
	return *this;
}
BigFigure& BigFigure::operator=(const double Source)
{
	this->toBF(NumStringDetail(Source));
	return *this;
}
BigFigure& BigFigure::operator=(const __int64 Source)
{
	this->toBF(NumStringDetail(Source));
	return *this;
}
BigFigure& BigFigure::operator=(const long Source)
{
	this->toBF(NumStringDetail(Source));
	return *this;
}
BigFigure& BigFigure::operator=(const int Source)
{
	this->toBF(NumStringDetail(Source));
	return *this;
}

std::ostream & operator<<(std::ostream & out, BigFigure & Source)
{
	out << Source.toString() << std::endl;
	return out;
}


/******************************************************************************************
对象方法
*******************************************************************************************/

//将字符串写入BF中
/*
已完成
*/
BigFigure& BigFigure::toBF(NumStringDetail &NumStringDetail) throw(...)
{
	int temp;
	char *tempStr, *SourceStr;
	size_t index_p, size;
	int skip;
	int expon;					//科学计数法的指数部分

	Detail->Minus = NumStringDetail.RadixMinus ? true : false;

	switch (NumStringDetail.Mode)
	{
	case 1://整数
	case 2://小数
		Detail->IntLen = NumStringDetail.IntLen;
		if (Detail->IntAllocatedLen >= NumStringDetail.IntLen)
		{
			//正常情况下
			if (NumStringDetail.IntBeZero)
			{
				//整数部分为0
				Detail->NumInt = Detail->IntTail - 1;
				Detail->NumInt[0] = '0';
				Detail->IntLen = 1;
			}
			else {
				//整数部分不为0
				Detail->NumInt = Detail->IntTail - NumStringDetail.IntLen;
				Detail->IntLen = NumStringDetail.IntLen;
				strncpy(Detail->NumInt, NumStringDetail.NumString.c_str() + NumStringDetail.IntStart_p, NumStringDetail.IntLen);
			}

		}
		else
		{
			//如果发生溢出
			if (ConfirmWontLossHighBit)
			{
				Detail->Illage = true;
				throw BFException(ERR_NUMBERTOOBIG, "对于目标对象,传入的数据太大,该对象无法容纳");
			}
			else
			{
				Detail->NumInt = Detail->StringHead;
				Detail->IntLen = Detail->IntAllocatedLen;
				temp = Detail->IntAllocatedLen - NumStringDetail.IntLen;
				strncpy(Detail->NumInt, NumStringDetail.NumString.c_str() + NumStringDetail.IntStart_p - temp, Detail->IntAllocatedLen);
			}
		}

		if (NumStringDetail.Mode == 1)
		{
			Detail->Illage = false;
			if (Detail->Accuracy)
				Detail->NumFloat[0] = 0;
			return *this;
		}
		//以下小数部分的处理
		if (Detail->Accuracy)
		{
			if (NumStringDetail.FloatLen <= Detail->Accuracy)
				strncpy(Detail->NumFloat, NumStringDetail.NumString.c_str() + NumStringDetail.FloatStart_p, NumStringDetail.FloatLen);
			else
			{
				if (ConfirmWontLossAccuracy)
				{
					Detail->Illage = true;
					throw BFException(ERR_MAYACCURACYLOSS, "写入的有效位数多于目标对象的容量,数据可能丢失");
				}
				else
					strncpy(Detail->NumFloat, NumStringDetail.NumString.c_str() + NumStringDetail.FloatStart_p, Detail->Accuracy);
			}
		}
		else
		{
			if (ConfirmWontLossAccuracy)							//小数位没有分配内存,将忽略小数位的处理
			{
				//确保不截断开关开启,抛出异常
				Detail->Illage = true;
				throw BFException(ERR_MAYACCURACYLOSS, "没有为小数位分配内存,可能会损失精度");
			}
		}
		Detail->Illage = false;
		return *this;
	case 3://科学计数法表示的以整数为底数的数字
	case 4://科学计数法表示的以小数为底数的数字
		tempStr = new char[NumStringDetail.NumString.length()]();				//临时存放有效位的字符串
		SourceStr = (char*)NumStringDetail.NumString.c_str() + NumStringDetail.FloatStart_p;	//存放小数位的首指针
		index_p = 0;

		//取出有效位
		if (NumStringDetail.Mode == 3)
		{
			//当底为整数时
			strncpy(tempStr, NumStringDetail.NumString.c_str() + NumStringDetail.IntStart_p, NumStringDetail.IntLen);
			expon = atoi(NumStringDetail.NumString.c_str() + NumStringDetail.ExpStart_p);
			index_p = NumStringDetail.IntLen - 1;
			skip = index_p;
			while (index_p != 0 && tempStr[index_p] == '0')tempStr[index_p--] = 0;//去除末尾的0
			size = index_p + 1;


			if (NumStringDetail.ExpMinus)
			{
				//指数为负数
				expon = ~expon + 1 + skip;
			}
			else
			{
				//指数为正数
				expon += skip;
			}
		}
		else
		{
			//当底为小数时
			if (!NumStringDetail.IntBeZero)
			{
				//整数部分不为0
				strncpy(tempStr, NumStringDetail.NumString.c_str() + NumStringDetail.IntStart_p, NumStringDetail.IntLen);	//复制整数部分的有效数字
				index_p = NumStringDetail.IntLen;
				skip = index_p - 1;
				strncpy(tempStr + index_p, SourceStr, NumStringDetail.FloatLen);		//复制小数部分的有效数字
				index_p += NumStringDetail.FloatLen;									//将小数位的长度加上,然后再去掉末尾的0的同时再减去表示0的长度
				while (index_p != 0 && tempStr[index_p] == '0')tempStr[index_p--] = 0;	//去除末尾的0
			}
			else {
				//整数部分为0
				index_p = NumStringDetail.FloatLen;
				skip = index_p;
				while (*(SourceStr) == '0')SourceStr++, index_p--;						//跳过数字前边的0
				skip = ~(skip - index_p + 1) + 1;										//取得跳过的有效位的位数
				strncpy(tempStr, SourceStr, index_p);									//复制有效数字
				while (index_p != 0 && tempStr[index_p] == '0')tempStr[index_p--] = 0;	//去除末尾的0
			}
			size = index_p + 1;
			expon = atoi(NumStringDetail.NumString.c_str() + NumStringDetail.ExpStart_p);
			if (NumStringDetail.ExpMinus)
			{
				//指数为负数
				expon = ~expon + 1 + skip;
			}
			else
			{
				//指数为正数
				expon += skip;
			}
		}

		//开始输入到对象中
		if (expon > 0)				//指数大于0的处理方案
		{
			if ((int)Detail->IntAllocatedLen > expon)
			{
				//空间足够存放整数数字
				Detail->IntLen = expon + 1;
				if (expon >= (int)size)
				{
					//如果指数大于有效位数的大小,此时小数点后一定为空,
					//不够的位要用0补齐
					Detail->NumInt = Detail->IntTail - expon;				//计算整数部分写入位置
					strcpy(Detail->NumInt, tempStr);						//复制有效位
					index_p = size;
					temp = Detail->IntLen - size;
					if (temp > 0)
						memset(Detail->NumInt + index_p - 1, '0', temp);		//空位用0填充
					Detail->NumFloat[0] = 0;
				}
				else
				{
					//指数大小小于有效位数,则只在整数部分输出expon个有效数字,其余的输出到小数位去
					Detail->NumInt = Detail->IntTail - expon - 1;				//计算写入位置
					strncpy(Detail->NumInt, tempStr, expon + 1);				//写入整数位
					if (Detail->Accuracy) {
						if (size - expon - 1 <= Detail->Accuracy)					//判断是否足够小数位存放
						{
							//小数位足够存放
							strcpy(Detail->NumFloat, tempStr + expon + 1);			//写入小数位,足够存放,直接复制
						}
						else
						{
							if (ConfirmWontLossAccuracy)							//小数位不足以存放,则考虑截断
							{
								//确保不截断开关开启,抛出异常
								Detail->Illage = false;
								throw BFException(ERR_MAYACCURACYLOSS, "对象分配的内存太小,不足以存储所以的有效位,可能会损失精度");
							}
							else
							{
								//进行截断处理
								strncpy(Detail->NumFloat, tempStr + expon, Detail->Accuracy);	//写入小数位,不够存放,进行截断
							}
						}
					}
					else
					{
						if (ConfirmWontLossAccuracy)							//小数位没有分配内存,将忽略小数位的处理
						{
							//确保不截断开关开启,抛出异常
							Detail->Illage = false;
							throw BFException(ERR_MAYACCURACYLOSS, "没有为小数位分配内存,可能会损失精度");
						}
					}
				}
			}
			else
			{
				//空间不足以存放整数数据
				if (ConfirmWontLossHighBit)
				{
					Detail->Illage = false;
					throw BFException(ERR_NUMBERTOOBIG, "数字太大,该对象无法存下这个数");
				}
				else
				{
					//截取整数低位的数据存入
					Detail->NumInt = Detail->StringHead;
					Detail->IntLen = Detail->IntAllocatedLen;
					temp = expon - Detail->IntAllocatedLen;			//忽略的有效位的偏移量
					strcpy(Detail->NumInt, tempStr + temp);
					memset(Detail->NumInt + size - temp - 1, '0', Detail->IntAllocatedLen - (size - temp));
				}
			}
		}
		else if (expon < 0)				//指数小于0时的处理方案
		{
			Detail->NumInt = Detail->IntTail - 1;
			Detail->NumInt[0] = '0';
			Detail->IntLen = 1;

			memset(Detail->NumFloat, '0', ~expon);
			index_p = ~expon;
			if (Detail->Accuracy >= size + index_p)
			{
				//精确度大于有效位数,能够完全存下
				strcpy(Detail->NumFloat + index_p, tempStr);
			}
			else {
				//小数位不够,有效位会被截断
				if (ConfirmWontLossAccuracy)
				{
					Detail->Illage = false;
					throw BFException(ERR_MAYACCURACYLOSS, "有效位数不够,可能丢失精度");
				}
				else
					strncpy(Detail->NumFloat + index_p, tempStr, Detail->Accuracy);
			}
		}
		else
		{
			//指数等于0的处理方案
			Detail->NumInt = Detail->IntTail - 1;
			Detail->NumInt[0] = tempStr[0];
			Detail->IntLen = 1;

			if (Detail->Accuracy >= size - 1)
			{
				//精确度大于有效位数,能够完全存下
				strcpy(Detail->NumFloat, tempStr + 1);
			}
			else {
				//小数位不够,有效位会被截断
				if (ConfirmWontLossAccuracy)
				{
					Detail->Illage = false;
					throw BFException(ERR_MAYACCURACYLOSS, "有效位数不够,可能丢失精度");
				}
				else
					strncpy(Detail->NumFloat, tempStr + 1, Detail->Accuracy);
			}
		}

		Detail->Illage = true;
		delete[] tempStr;
		return *this;
	case 0:
	default:
		Detail->Illage = false;
		throw BFException(ERR_ILLEGALNUMBER, "不是一个合法的数字");
		break;
	}
	return *this;
}

//将对象中存储的数据转化为字符串,并返回(使用全局设定进行输出)
std::string BigFigure::toString()
{
	return toString(ScinotationShow, ReserveZero);
}

//将对象中存储的数据转化为字符串,并返回
/*
Mode 0
Mode 1 按照普通的输出方式进行输出(默认)
Mode 2 按照科学计数法进行输出
*/
std::string BigFigure::toString(bool UseScinotation, bool ReserveZero)
{
	char *tempString = NULL;							//保存缓冲区地址
	int skip;											//被跳过的位数省略的位数用于最终计算指数
	bool HasNumPre;
	std::string RetVal;									//返回的字符串的长度
	size_t index_p = 0;									//写入位置标记
	size_t r_index;										//从元数据中读取数据的下标

	if (Detail->Illage)
	{
		//非法的数字输出Nan
		return std::string("NaN");
	}
	else {
		tempString = new char[Detail->AllocatedSize];	//新建一块缓冲区
		if (Detail->Minus)								//输出负号
			tempString[index_p++] = '-';

		if (UseScinotation)
		{
			//使用科学计数法输出

			if (Detail->NumInt[0] == '0' && Detail->IntLen == 1)
			{
				//整数部分数据为0
				HasNumPre = false;
				r_index = 0;
				skip = 0;

				while (Detail->NumFloat[r_index] == '0' && r_index < Detail->Accuracy) r_index++;	//找到有效位

				if (Detail->NumFloat[r_index] != 0)
				{
					skip = r_index - 1;						//计算省略的位数
					tempString[index_p++] = Detail->NumFloat[r_index++];	//输出第一位
					if (Detail->NumFloat[r_index] != 0)
					{
						//后面还有有效位,继续输出
						tempString[index_p++] = '.';						//输出小数点
						while (Detail->NumFloat[r_index] != 0 && r_index < Detail->Accuracy&&index_p < ScinotationLen)	//输出剩余有效位
							tempString[index_p++] = Detail->NumFloat[r_index++];
						if (!ReserveZero)
						{
							index_p--;
							while (tempString[index_p] == '0')index_p--;	//将有效数字尾部的'0'去除
							tempString[index_p++] = 0;						//写入'\0'
						}
					}
				}
				else
				{
					//该值为0,写入0
					tempString[index_p++] = '0';
					tempString[index_p++] = 0;
				}
			}
			else
			{
				//整数部分数据不为0
				HasNumPre = true;
				skip = Detail->IntLen - 1;
				tempString[index_p++] = Detail->NumInt[0];							//把第一位输入
				if (Detail->NumInt[index_p] != 0)
				{
					tempString[index_p++] = '.';
					strncpy(tempString + index_p, Detail->NumInt + 1, ScinotationLen - 1);
					if (ScinotationLen <= Detail->IntLen)
					{
						//精度大于有效位
						index_p += ScinotationLen - 1;
					}
					else
					{
						//有效位大于精度
						index_p += Detail->IntLen - 1;
					}
				}
				else if (Detail->Accuracy &&Detail->NumFloat[0] != 0)
					tempString[index_p++] = '.';//小数点后有数字

				if (Detail->Accuracy &&Detail->NumFloat[0] != 0)
				{
					if (Detail->IntLen < ScinotationLen)
					{
						//整数的有效位不够,继续拿小数位
						strncpy(tempString + index_p, Detail->NumFloat, ScinotationLen - Detail->IntLen);
						index_p += ScinotationLen - Detail->IntLen;
					}
					if (!ReserveZero)
					{
						index_p--;
						while (tempString[index_p] == '0')index_p--;	//将有效数字尾部的'0'去除
						tempString[index_p++] = 0;						//写入'\0'
					}
				}
			}

			if (skip) //输出指数
			{
				tempString[index_p++] = 'E';
				sprintf(tempString + index_p, "%d", skip);
			}
		}
		else {
			//正常数字输出
			strncpy(tempString + index_p, Detail->NumInt, Detail->IntLen);

			index_p += Detail->IntLen;
			if (Detail->Accuracy)
			{
				r_index = 0;
				if (Detail->NumFloat[0] != 0)
					tempString[index_p++] = '.';
				while (Detail->NumFloat[r_index] != 0)
					tempString[index_p++] = Detail->NumFloat[r_index++];

				if (!ReserveZero)									//去除0
				{
					index_p--;										//指向上一个已写的位
					while (tempString[index_p] == '0') index_p--;	//删除为0的位
					if (tempString[index_p] == '.')index_p--;		//如果小数点被删光,则再删除小数点
					index_p++;										//指向下一个可写的位
				}
			}
			tempString[index_p] = 0;								//写入字符串结束符
		}
		RetVal = std::string(tempString);
		delete tempString;
	}
	return RetVal;
}

//将一个对象的值复制到当前对象中,两个对象相互独立
BigFigure & BigFigure::CopyDetail(const BigFigure & Source)
{
	this->Detail->Minus = Source.Detail->Minus;
	this->Detail->Illage = Source.Detail->Illage;
	if (this->Detail->IntAllocatedLen >= Source.Detail->IntLen)
	{
		//空间足够复制,进行复制
		this->Detail->NumInt = this->Detail->IntTail - Source.Detail->IntLen;	//找到写入位置
		this->Detail->IntLen = Source.Detail->IntLen;
		strcpy(this->Detail->NumInt, Source.Detail->NumInt);
	}
	else
	{
		//空间不足以复制,进行判断,考虑截断
		if (ConfirmWontLossHighBit)
			throw BFException(ERR_NUMBERTOOBIG, "数据溢出");
		else
		{
			this->Detail->NumInt = this->Detail->StringHead;
			strncpy(this->Detail->NumInt, Source.Detail->NumInt + Source.Detail->IntLen - this->Detail->IntAllocatedLen, this->Detail->IntAllocatedLen);
			while (this->Detail->NumInt[0] == '0')this->Detail->NumInt++;		//去除整数前面的0
			this->Detail->IntLen = this->Detail->IntTail - this->Detail->NumInt;//计算整数的长度
		}
	}


	if (this->Detail->Accuracy) {
		if (Source.Detail->Accuracy)
		{
			if (Source.Detail->NumFloat[0] != 0)
			{
				if (this->Detail->Accuracy >= Source.Detail->Accuracy)
				{
					//一定装得下小数位
					strcpy(this->Detail->NumFloat, Source.Detail->NumFloat);
				}
				else
				{
					size_t len = strlen(Source.Detail->NumFloat);
					if (len > this->Detail->Accuracy)
					{
						//小数位超过,将会截断
						if (ConfirmWontLossAccuracy)
							throw BFException(ERR_MAYACCURACYLOSS, "源数据的大小比目标数据的大小大,可能丢失精度");
						else
						{
							//进行截断
							strncpy(this->Detail->NumFloat, Source.Detail->NumFloat, this->Detail->Accuracy);
						}
					}
					else {
						strcpy(this->Detail->NumFloat, Source.Detail->NumFloat);
					}
				}
			}
		}
		else {
			//源小数位无数据
			this->Detail->NumFloat[0] = 0;
		}
	}
	else {
		//如果目标对象没有分配整数位,不需要进行复制,但是要判断情况,决定是否要抛出异常
		if (ConfirmWontLossAccuracy)
		{
			if (Source.Detail->Accuracy)
			{
				if (Source.Detail->NumFloat[0] != 0)
					throw BFException(ERR_MAYACCURACYLOSS, "目标对象没有小数位,源数据的小数位将被忽略");
				//提示可能丢失精度
			}
		}
	}
	return *this;
}


/******************************************************************************************
基础函数
*******************************************************************************************/

//检查字符串是否为数字,并且判断字符串表示的是整数还是小数,并且输出长度信息
/*
返回值代表的含义如下:
  1.整数
  2.小数
  3.底数为整数的科学计数法表示的数
  4.底数为小数的科学计数法表示的数
  0.非数字
*/
bool NumCheck(NumStringDetail &NumDetail)
{
	const char *tempString = NumDetail.NumString.c_str();
	size_t StringLen = NumDetail.NumString.length();
	bool HasNumPre = false;			//记录在指定符号前方是否有数字
	bool HasPoint = false;			//记录是否有小数点
	bool Scinotation = false;		//记录是否以科学计数法进行标记
	bool IntBeZero = true;			//记录整数部分是否为0,用于决定浮点的下标是否等找到有效位再设置
	size_t index_p;					//用此变量来记录所在字符串的位置,从而遍历字符串

	//开始遍历
	for (index_p = 0; index_p < StringLen; index_p++)
	{
		//开始进行判断
		if (tempString[index_p] > '0'&&tempString[index_p] <= '9')
		{
			if (!HasNumPre)
			{
				if (Scinotation)
					NumDetail.ExpStart_p = index_p;
				else
					/*
					if (HasPoint)
					{
						//小数点部分,设置小数点后字符串的指针
						//NumDetail.FloatStart_p = index_p;
					}
					else
					*/if (!HasPoint&&IntBeZero)
					{
						//整数部分,设置第一个有效数字的字符串的指针
						NumDetail.IntStart_p = index_p;
						IntBeZero = false;
					}

			}
			HasNumPre = true;
			continue;
		}
		else if (tempString[index_p] == '0')
		{
			//if (HasPoint && !(IntBeZero || HasNumPre))

			if (HasPoint && !HasNumPre)
			{
				//整数不为0,且前面没有数字(当整数有有效数字时,小数点后的0不能省略)
				HasNumPre = true;
			}
			continue;
		}
		else if (tempString[index_p] == '-')
		{
			if (HasNumPre)
			{
				//将负号掺杂在数字中间,数字无效
				NumDetail.Mode = 0;
				return 0;
			}
			if (Scinotation)
			{
				if (NumDetail.ExpMinus)
				{
					//在指数部分已有负号时(存在两个负号),数字无效
					NumDetail.Mode = 0;
					return 0;
				}
				else
					NumDetail.ExpMinus = 1;
				NumDetail.ExpStart_p = index_p + 1;
			}
			else {
				//处理底数部分
				if (NumDetail.RadixMinus)
				{
					//在底数部分已有负号时(存在两个负号),数字无效
					NumDetail.Mode = 0;
					return 0;
				}
				else
					NumDetail.RadixMinus = 1;
				NumDetail.IntStart_p = index_p + 1;
			}
			continue;
		}
		else if (tempString[index_p] == '.')
		{
			if (Scinotation) {
				//指数部分有小数点,数字无效
				NumDetail.Mode = 0;
				return 0;
			}
			else {
				if (!HasPoint)
				{
					//保存整数的长度
					NumDetail.IntLen = index_p - NumDetail.IntStart_p;
					if (!HasNumPre)
					{
						IntBeZero = true;
						NumDetail.IntLen = 0;
					}
					NumDetail.FloatStart_p = index_p + 1;
					HasPoint = true;
					HasNumPre = false;
				}
				else
				{
					//发现两个小数点,数字无效
					NumDetail.Mode = 0;
					return 0;
				}
			}
			continue;
		}
		else if (tempString[index_p] == 'E' || tempString[index_p] == 'e')
		{
			if (Scinotation)
			{
				//一个数字出现两个E,数字错误
				NumDetail.Mode = 0;
				return 0;
			}
			else {
				if (HasPoint)
				{
					//底数为小数
					//保存小数的长度
					if (HasNumPre)
						NumDetail.FloatLen = index_p - NumDetail.FloatStart_p;
					else
						HasPoint = false;
				}
				else
				{
					//在底数为整数
					//保存整数的长度
					NumDetail.IntLen = index_p - NumDetail.IntStart_p;
				}
				Scinotation = true;
				HasNumPre = false;
			}
			continue;
		}
		else if (tempString[index_p] == '+')
		{
			if (HasNumPre)
			{
				//将正号掺杂在数字中间,数字无效
				NumDetail.IntStart_p = index_p + 1;
				NumDetail.Mode = 0;
				return 0;
			}
			continue;
		}
		else {
			//发现其他字符,数字非法
			NumDetail.Mode = 0;
			return 0;
		}
	}

	if (!HasNumPre && !HasPoint)
	{
		//单独处理0
		NumDetail.IntStart_p = 0;
		NumDetail.IntLen = 1;
		if (NumDetail.NumString.empty())	//为空字符串时把字符串初始化为0
			NumDetail.NumString = "0";
	}

	//如果是正确的数字, 则可以通过这个for的验证
	NumDetail.IntBeZero = IntBeZero;
	if (Scinotation)
	{
		if (HasNumPre)
		{
			NumDetail.Mode = 2;		//初始化为2,Mode将为3,4
			NumDetail.ExpLen = index_p - NumDetail.ExpStart_p;
		}
		else
		{
			NumDetail.Mode = 0;		//初始化为0,Mode将为1,2
		}
	}
	else
	{
		NumDetail.Mode = 0;		//初始化为0,Mode将为1,2
		if (HasPoint)
			NumDetail.FloatLen = index_p - NumDetail.FloatStart_p;
		else
			NumDetail.IntLen = index_p - NumDetail.IntStart_p;
	}

	if (HasPoint)
	{
		if (NumDetail.FloatLen)
		{
			NumDetail.Mode += 2;
		}
		else
		{
			NumDetail.Mode += 1;
		}

	}
	else {
		NumDetail.Mode += 1;
		//NumDetail.IntLen = index_p - NumDetail.IntStart_p;
	}

	return 1;
}

//比较两个数的大小
/*
返回值表示两个数的大小情况
0,	OperandA==OperandB
1,	OperandA>OperandB
-1,	OperandA<OperandB
2,	OperandA>>OperandB
-2,	OperandA<<OperandB
*/
int BFCmp(const BigFigure &OperandA, const BigFigure &OperandB)
{
	int minus = 1;				//当为负数时,返回的结果与正数相反,此变量控制的就是这个因数

	//判断负号
	if (OperandA.Detail->Minus)
	{
		if (OperandB.Detail->Minus)
			minus = -1;//A,B同负
		else
			return -2;//A负B正
	}
	else
	{
		if (OperandB.Detail->Minus)
			return 2;//A正B负
		//minus默认为1,不需要改动
	}

	//判断位
	if (OperandA.Detail->IntLen > OperandB.Detail->IntLen)
	{
		return 2 * minus;
	}
	else if (OperandA.Detail->IntLen < OperandB.Detail->IntLen)
	{
		return -2 * minus;
	}
	else
	{
		//如果两个数整数部分长度相等,则进行比较各个位
		int temp = strcmp(OperandA.Detail->NumInt, OperandB.Detail->NumInt);
		if (temp)
		{
			//比较出结果,返回
			return temp*minus;
		}
		else {
			//整数部分完全相等,继续进行比较小数部分
			int index_p = 0;
			while (OperandA.Detail->NumFloat[index_p] && OperandA.Detail->NumFloat[index_p] == OperandB.Detail->NumFloat[index_p])index_p++;
			if (OperandB.Detail->NumFloat[index_p] && OperandA.Detail->NumFloat[index_p] > OperandB.Detail->NumFloat[index_p])
			{
				//if (OperandB.Detail->NumFloat[index_p] >= '0')
				return minus;
			}
			else if (OperandA.Detail->NumFloat[index_p] && OperandA.Detail->NumFloat[index_p] < OperandB.Detail->NumFloat[index_p])
			{
				return -minus;
			}
			else
			{
				temp = 0;
				int index_p2 = index_p;
				if (OperandA.Detail->NumFloat[index_p2] == '0')
				{
					index_p2++;
					while (OperandA.Detail->NumFloat[index_p2] == '0'&&OperandA.Detail->NumFloat[index_p2] != 0)index_p2++;
					if (OperandA.Detail->NumFloat[index_p2] != 0 && OperandA.Detail->NumFloat[index_p2] != '0')
						temp = 1;
				}
				index_p2 = index_p;
				if (OperandB.Detail->NumFloat[index_p2] == '0')
				{
					index_p2++;
					while (OperandB.Detail->NumFloat[index_p2] == '0'&&OperandB.Detail->NumFloat[index_p2] != 0)index_p2++;
					if (OperandB.Detail->NumFloat[index_p2] != 0 && OperandB.Detail->NumFloat[index_p2] != '0')
						temp = -1;
				}
				return temp*minus;
			}
		}
	}
}
