#ifndef BigNumber_h
#define BigNumber_h

#include <iostream>
#include <string>

//#define DEFAULT_ACCURACY 8      //默认小数的精度保留8位,如果有更高精度的需求,可修改它

//定义超出长度的数字的处理方式
//1.截断
//2.抛出异常
//3.改变对象（重新分配）

//错误值的定义
#define ERR_UNKNOWEXCEPTION 0
#define ERR_ARRAYINDEXOUTOFBOUNDSEXCEPTION 1
#define ERR_MEMORYALLOCATEDEXCEPTION 2
#define ERR_NULLPOINTEXCEPTION 3
#define ERR_ILLEGALNUMBER 4
#define ERR_DIVISORCANNOTBEZERO 5
#define ERR_ILLEGALPARAM 6
#define ERR_NUMBERTOOBIG 7
#define ERR_MAYACCURACYLOSS 8

//编译开关



//全局变量
extern bool ReserveZero;			//是否保留小数点后的0
extern bool ScinotationShow;		//是否以科学计数法显示数字
extern bool ConfirmWontLossAccuracy;//确保不截断小数的有效位(关闭之后如果赋值时小数位太多,则会省略多出的部分;如果开启,则会抛出异常)
extern size_t ScinotationLen;		//用小数点表示时的有效位数

class BFException
{
private:
	std::string message;
public:
	BFException(int ErrVal, std::string detail);
	~BFException();
	//void GetMessage();       //输出错误信息
};


class BigFigure
{
private:
	struct BFDetail
	{
		size_t ReferCount;      //引用计数,用于判断何时销毁
		size_t AllocatedSize;   //数字字符串分配的大小
		size_t Accuracy;        //浮点数部分的最大精确度
		size_t IntLen;			//整数部分数字的有效位数(实际长度)
		size_t IntAllocatedLen;	//整数部分的已分配内存长度
		bool Minus;             //表示是否为负数,如果为负数,则该值为1
		char *NumInt;           //可输出的整数部分的字符串的首地址
		char *NumFloat;         //可输出的浮点数部分的字符串的首地址
		char *StringHead;       //保存申请的字符串空间的首指针

	} *Detail;
	//void FreeDetail();
public:
	BigFigure();
	BigFigure(size_t IntSize, size_t FloatSize)throw(...);
	BigFigure(const BigFigure& Base);
	~BigFigure();
	void core_IntAdd(BigFigure &result, BigFigure &OperandA, BigFigure &OperandB);
	//void core_IntAdd(BigFigure &result, BigFigure &OperandA, int OperandB);
	//void core_FloatAdd();
	//void core_FloatAdd();
	void atoBF(std::string NumString);
	void printfBF();
	void printDetail();

};

typedef BigFigure BF;

/*
全局基础函数
*/
int NumCheck(std::string CheckString);							//检查字符串是否为合法数字,并返回数字的类型
int BitCount(std::string NumString, int NumType, int result[2]);//计算数字的整数位的长度和小数位的长度
int BitCount_check(std::string NumString, int result[2]);		//检查数字的合法性并计算整数位和小数位的长度

#endif
