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
#define ERR_UNKNOWEXCEPTION 0				//未知的错误
#define ERR_ARRAYINDEXOUTOFBOUNDSEXCEPTION 1//数组下标越界
#define ERR_MEMORYALLOCATEDEXCEPTION 2		//内存分配错误
#define ERR_NULLPOINTEXCEPTION 3			//空指针
#define ERR_ILLEGALNUMBER 4					//非法的数字
#define ERR_DIVISORCANNOTBEZERO 5			//除数不能为0
#define ERR_ILLEGALPARAM 6					//参数有误
#define ERR_NUMBERTOOBIG 7					//源数据大小大于目标容量
#define ERR_MAYACCURACYLOSS 8				//可能丢失精度(小数点后的位被舍弃,并且ConfirmWontLossAccuracy==true)时才抛出
//编译开关



//全局变量
extern bool ReserveZero;			//是否保留小数点后的0
extern bool ScinotationShow;		//是否以科学计数法显示数字
extern bool ConfirmWontLossAccuracy;//确保不截断小数的有效位(关闭之后如果赋值时小数位太多,则会省略多出的部分;如果开启,则会抛出异常)
extern bool ConfirmWontLossHighBit;	//确保不丢失整数的高位(如果发生溢出时),如果为true,则发生溢出时会抛出异常,如果为false,则如果溢出则舍弃高位
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

//用于存储字符串数字字符串的信息
class NumStringDetail
{
private:
	typedef char byte;
	byte RadixMinus : 1;	//底数部分是否有负号
	byte ExpMinus : 1;		//指数部分是否有负号
	byte IntBeZero : 1;		//整数部分是否为0,决定是否需要在浮点数中寻找有效位
	byte Mode : 4;			//表示的数字的类型
	size_t IntLen;			//整数部分的长度
	size_t IntStart_p;		//整数的起始下标
	size_t FloatLen;		//浮点数部分的长度
	size_t FloatStart_p;	//浮点数的起始下标
	size_t ExpLen;			//指数部分的长度
	size_t ExpStart_p;		//指数的起始下标
	std::string NumString;	//保存的字符串信息
public:
	NumStringDetail(std::string NumString);
	NumStringDetail(double Num);
	NumStringDetail(__int64 Num);
	NumStringDetail(long Num);
	NumStringDetail(int Num);
	friend bool NumCheck(NumStringDetail &NumDetail);
	friend class BigFigure;
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
		bool Illage;			//表示是否为非法数字,如果是非法数字,则此值为true
		char *NumInt;           //可输出的整数部分的字符串的首地址
		char *IntTail;			//整数部分的尾地址(用于快速计算写入位置)(处于小数点所在的位置)
		char *NumFloat;         //可输出的浮点数部分的字符串的首地址
		char *StringHead;       //保存申请的字符串空间的首指针
	} *Detail;
public:
	//构造器与析构器
	//BigFigure();
	BigFigure(size_t IntSize, size_t FloatSize)throw(...);
	BigFigure(const BigFigure& Base);
	~BigFigure();

	//运算函数

	friend void core_IntAdd(BigFigure & result, const BigFigure & OperandA, const BigFigure & OperandB, int carry);
	template <class T> friend void core_IntAdd_Basis(BigFigure & result, const BigFigure & OperandA, T OperandB, int carry);
	friend int core_FloatAdd(BigFigure & result, const BigFigure & OperandA, const BigFigure & OperandB);
	friend void core_IntSub(BigFigure & result, const BigFigure & OperandA, const BigFigure & OperandB, int borrow);
	friend void core_FloatSub(BigFigure & result, const BigFigure & OperandA, const BigFigure & OperandB);

	//friend void core_IntAdd(BigFigure & result, const BigFigure & OperandA, double OperandB);
	friend void core_IntAdd(BigFigure & result, const BigFigure & OperandA, __int64 OperandB);
	friend void core_IntAdd(BigFigure & result, const BigFigure & OperandA, long OperandB);
	friend void core_IntAdd(BigFigure & result, const BigFigure & OperandA, int OperandB);
	//friend void core_FloatAdd(BigFigure & result, const BigFigure & OperandA, double OperandB);

	friend void IntAdd(BigFigure & result, BigFigure & OperandA, BigFigure & OperandB);
	friend void IntAdd(BigFigure & result, BigFigure & OperandA, double OperandB);


	//void core_IntAdd(BigFigure &result, BigFigure &OperandA, int OperandB);
	//void core_FloatAdd();
	//void core_FloatAdd();

	//基础函数
	BigFigure& toBF(NumStringDetail &NumStringDetail);						//将其他数据类型写入到当前对象中
	std::string toString();													//将当前对象输出为字符串
	std::string toString(bool UseScinotation, bool ReserveZero);			//将当前对象输出为字符串(通过mode可选择输出模式(正常显示|科学计数法))
	BigFigure& CopyDetail(const BigFigure &Source);							//复制数字的值
	void printDetail();														//打印数字的详细信息

	//友元函数
	friend int BFCmp(const BigFigure &OperandA, const BigFigure &OperandB);	//比较两个数字的大小

	//重载函数
	BigFigure& operator=(const BigFigure &Source);
	BigFigure& operator=(const char* Source);
	BigFigure& operator=(const double Source);
	BigFigure& operator=(const __int64 Source);
	BigFigure& operator=(const long Source);
	BigFigure& operator=(const int Source);
	friend std::ostream& operator<<(std::ostream& out, BigFigure& Source);
	friend bool operator<(const BigFigure& OperandA, const BigFigure&OperandB);
	friend bool operator>(const BigFigure& OperandA, const BigFigure&OperandB);
	friend bool operator==(const BigFigure& OperandA, const BigFigure&OperandB);
	friend bool operator<=(const BigFigure& OperandA, const BigFigure&OperandB);
	friend bool operator>=(const BigFigure& OperandA, const BigFigure&OperandB);
};

typedef BigFigure BF;

/*
全局基础函数
*/
bool NumCheck(NumStringDetail &NumDetail);									//检查字符串是否为合法数字,并返回数字的类型
#endif

