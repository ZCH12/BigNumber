#include <memory>
#include <cstdio>
#include <sstream>
//默认的长度分配内存的大小

/*
 超长数字运算库
 待实现：加减乘除模，
*/

#include "BigNumber.h"

//控制显示方式的变量
bool ReserveZero = true;			//保留小数后的0
bool ScinotationShow = false;		//不以科学计数法显示数字
bool ConfirmWontLossAccuracy = true;	//确保不截断小数的有效位(关闭之后如果赋值时小数位太多,则会省略多出的部分;如果开启,则会抛出异常)
size_t ScinotationLen = 5;			//科学计数法时有效位数为5位


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
		Detail->IntAllocatedLen = IntSize;
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

#if 0
//将字符串转换为BF类型,并保存在当前this对象中
/*
#######未完成
*/
void BigFigure::atoBF(std::string NumString)
{
	int NumType;
	const char *tempString = NumString.c_str();
	size_t len, len2;
	long exponent;			//用于科学计数法的计算位数,保存科学计数法的指数部分
	int NumLen[2] = { 0 };
	int NumPart = 0;
	size_t r_p = 0, w_p, e_p = 0;	//记录科学计数法时两个下标(一个读的下标,一个写的下标),e_p是E的下标
	NumType = NumCheck(NumString);					//检查数字类型
	NumPart = BitCount(NumString, NumType, NumLen);	//计算长度
	if (NumPart > 1)	//内存不足阻止执行
	{
		if (NumLen[0] > Detail->IntAllocatedLen)
		{
			throw BFException(ERR_NUMBERTOOBIG, "已分配的内存无法存放传入的变量");
		}
		else if (NumLen[1] > Detail->Accuracy)
		{
			if (ConfirmWontLossAccuracy)
				throw BFException(ERR_MAYACCURACYLOSS, "该对象设置的精度太小,可能丢失精度");
		}
	}

	Detail->Minus = false;
	switch (NumType)
	{
	case -1://负整数
		Detail->Minus = true;
		Detail->IntLen = NumString.length() - 1;
		Detail->NumInt = Detail->StringHead + Detail->IntAllocatedLen - NumString.length();
		strcpy(Detail->NumInt, tempString + 1);											//复制除负号外的字符
		break;
	case 1://正整数
		Detail->Minus = false;
		Detail->IntLen = NumString.length();											//计算整数部分长度
		Detail->NumInt = Detail->StringHead + Detail->IntAllocatedLen - NumString.length();
		strcpy(Detail->NumInt, tempString);												//复制整个字符串
		break;
	case -2://负小数
		Detail->Minus = true;
		len = 0;
		while (tempString[len] != '.')len++;		//找到小数点所在的位置,len代表小数点是第几个字符(从1开始计数)
		Detail->NumInt = Detail->StringHead + Detail->IntAllocatedLen - len - 1;			//计算写入位置,修改整数部分字符串的指针
		strncpy(Detail->NumInt, tempString + 1, len - 1);									//复制整数部分
		Detail->IntLen = len;															//保存整数部分的长度
		strncpy(Detail->NumFloat, tempString + len + 1, Detail->Accuracy);				//复制小数部分
		break;
	case 2://正小数
		len = 0;
		Detail->Minus = false;
		while (tempString[len] != '.')len++;		//找到小数点所在的位置,len代表小数点是第几个字符(从1开始计数)
		Detail->NumInt = Detail->StringHead + Detail->IntAllocatedLen - len;			//计算写入位置,修改整数部分字符串的指针
		strncpy(Detail->NumInt, tempString, len);										//复制整数部分
		Detail->IntLen = len;															//保存整数部分的长度
		strncpy(Detail->NumFloat, tempString + len + 1, Detail->Accuracy);				//复制小数部分
		break;
	case -30://负科学计数数(正指数)
		r_p = 1;
		Detail->Minus = true;															//保存负号
		Detail->NumInt = Detail->StringHead + Detail->IntAllocatedLen - NumLen[0];		//找到写入位置
		while (tempString[e_p] != 'E' && tempString[e_p] != 'e') e_p++;					//找到E的位置
		while ((tempString[r_p] == '0' || tempString[r_p] == '.') && r_p < e_p) r_p++;	//找到第一个有效数字
		NumLen[0] -= r_p;
		if (tempString[r_p] == 'E' || tempString[r_p] == 'e')
		{
			//数字为0
			Detail->NumInt[0] = '0';
			Detail->NumFloat[0] = 0;
			break;
		}
		else {
			//数字不为0,开始进行复制
			if (NumLen[0] > 0)
			{
				//大于0的数
				w_p = 0;
				while (w_p < NumLen[0] && r_p < e_p)
				{
					if (tempString[r_p] >= '0'&&tempString[r_p] <= '9')
						Detail->NumInt[w_p++] = tempString[r_p++];
					else
						r_p++;
				}
				if (r_p < e_p)
					while (w_p < NumLen[0])
						Detail->NumInt[w_p++] = '0';
				w_p = 0;
				while (w_p < NumLen[1] && r_p < e_p)
				{
					if (tempString[r_p] >= '0'&&tempString[r_p] <= '9')
						Detail->NumFloat[w_p++] = tempString[r_p++];
					else
						r_p++;
				}
				if (r_p < e_p)
					while (w_p < NumLen[1])
						Detail->NumFloat[w_p++] = '0';
			}
			else
			{
				//小于0的数
				Detail->NumInt[0] = '0';
				w_p = 0;
				for (NumLen[0]++; NumLen[0] < 0; NumLen[0]++)
				{
					Detail->NumFloat[w_p++] = '0';
				}
				while (w_p < NumLen[1] && r_p < e_p)
				{
					if (tempString[r_p] >= '0'&&tempString[r_p] <= '9')
						Detail->NumFloat[w_p++] = tempString[r_p++];
					else
						r_p++;
				}
			}

		}
		break;
	case 30://正科学计数数(正指数)
		r_p = 0;
		Detail->Minus = false;															//保存负号
		Detail->NumInt = Detail->StringHead + Detail->IntAllocatedLen - NumLen[0];		//找到写入位置
		while (tempString[e_p] != 'E' && tempString[e_p] != 'e') e_p++;					//找到E的位置
		while ((tempString[r_p] == '0' || tempString[r_p] == '.') && r_p < e_p) r_p++;	//找到第一个有效数字
		NumLen[0] -= r_p;
		if (tempString[r_p] == 'E' || tempString[r_p] == 'e')
		{
			//数字为0
			Detail->NumInt[0] = '0';
			Detail->NumFloat[0] = 0;
			break;
		}
		else {
			//数字不为0,开始进行复制
			if (NumLen[0] > 0)
			{
				//大于0的数
				w_p = 0;
				while (w_p < NumLen[0] && r_p < e_p)
				{
					if (tempString[r_p] >= '0'&&tempString[r_p] <= '9')
						Detail->NumInt[w_p++] = tempString[r_p++];
					else
						r_p++;
				}
				if (r_p < e_p)
					while (w_p < NumLen[0])
						Detail->NumInt[w_p++] = '0';
				w_p = 0;
				while (w_p < NumLen[1] && r_p < e_p)
				{
					if (tempString[r_p] >= '0'&&tempString[r_p] <= '9')
						Detail->NumFloat[w_p++] = tempString[r_p++];
					else
						r_p++;
				}
				if (r_p < e_p)
					while (w_p < NumLen[1])
						Detail->NumFloat[w_p++] = '0';
			}
			else
			{
				//小于0的数
				Detail->NumInt[0] = '0';
				w_p = 0;
				for (NumLen[0]++; NumLen[0] < 0; NumLen[0]++)
				{
					Detail->NumFloat[w_p++] = '0';
				}
				while (w_p < NumLen[1] && r_p < e_p)
				{
					if (tempString[r_p] >= '0'&&tempString[r_p] <= '9')
						Detail->NumFloat[w_p++] = tempString[r_p++];
					else
						r_p++;
				}
			}

		}
		break;

	case -31://负科学计数数(负指数)
		Detail->Minus = true;															//保存负号
		while (tempString[e_p] != 'E' && tempString[e_p] != 'e') e_p++;					//找到E的位置
		exponent = atol(tempString + e_p + 1);											//把指数转化为long类型
		while (tempString[r_p] == '0' && r_p < e_p) r_p++;								//找到第一个有效数字

		break;

	case 31://正科学计数数(负指数)
		Detail->Minus = false;

		break;
	default:
	case 0:
		throw BFException(ERR_ILLEGALNUMBER, "字符串表示的不是一个合法的数字");
		break;
	}
	return;
}

//显示当前对象存储的数字
/*
已完成,待检验
有bug
*/
void BigFigure::printfBF()
{
	size_t unPrintZero = 0;		//记录还未显示出来的0的个数
	size_t a = 0;
	bool hasPoint = false;
	if (Detail->Minus)
		printf("-");
	if (ScinotationShow)
	{
		//以科学计数法显示
		size_t nowLen = 1;		//统计已输出的有效位的个数
		size_t skip = 0;

		while (Detail->NumInt[a] == '0')
			a++, skip++;
		if (Detail->NumInt[a] == 0)
		{
			//整数部分已经为0,已经被跳过
			a = 0;
			while (Detail->NumFloat[a] == '0')
				a++, skip++;
			if (Detail->NumFloat[a] == 0)
			{
				//发现全部项都为0,数字为0
				skip = 0;
				printf("0\n");
				return;
			}
			else {
				printf("%c", Detail->NumFloat[a]);
				a++;
			}
			//显示剩余的应显示的位
			while (!hasPoint&& nowLen < ScinotationLen&&Detail->NumFloat[a] != 0)
			{
				if (Detail->NumFloat[a] == '0')
					unPrintZero += 1;
				else
				{
					if (unPrintZero != 0)
					{
						printf("%s%0*d", hasPoint ? "" : ".", unPrintZero, 0);
						unPrintZero = 0;
					}
					printf("%s%c", hasPoint ? "" : ".", Detail->NumFloat[a++]);
					hasPoint = true;
				}
				nowLen++;
			}
			while (nowLen < ScinotationLen&&Detail->NumFloat[a] != 0)
			{
				if (Detail->NumFloat[a] == '0')
					unPrintZero += 1;
				else
				{
					if (unPrintZero != 0)
					{
						printf("%0*d", unPrintZero, 0);
						unPrintZero = 0;
					}
					printf("%c", Detail->NumFloat[a++]);
				}
				nowLen++;
			}
			//用0补齐有效位数


			if (ReserveZero) {
				printf("%s", hasPoint ? "" : ".");
				//hasPoint=0;
				if (Detail->NumFloat[a] == 0)
					for (; nowLen < ScinotationLen; nowLen++)
						printf("0");
			}
			printf("E%s%d\n", skip ? "-" : "+", skip);
		}
		else {
			//整数部分不为0,输出第一个有效位,接着输出第二个有效位
			printf("%c", Detail->NumInt[a++]);
			skip = 0;			//在这里skip用于存储在整数位后被移动到小数位的位数

			while (!hasPoint&&nowLen < ScinotationLen&&Detail->NumInt[a] != 0)	//等待需要小数点的地方
			{
				if (!ReserveZero) {
					//不保留末尾的0
					if (Detail->NumInt[a] == '0')
						unPrintZero += 1;
					else
					{
						if (unPrintZero != 0)
						{
							printf("%s%0*d", hasPoint ? "" : ".", unPrintZero, 0);
							unPrintZero = 0;
							hasPoint = true;
						}
						printf("%s%c", hasPoint ? "" : ".", Detail->NumInt[a]);
						hasPoint = true;
					}
				}
				else {
					printf("%s%c", hasPoint ? "" : ".", Detail->NumInt[a]);
					hasPoint = true;
				}
				nowLen++, a++, skip++;
			}
			while (nowLen < ScinotationLen&&Detail->NumInt[a] != 0)
			{
				if (!ReserveZero) {
					//不保留末尾的0
					if (Detail->NumInt[a] == '0')
						unPrintZero += 1;
					else
					{
						if (unPrintZero != 0)
						{
							printf("%0*d", unPrintZero, 0);
							unPrintZero = 0;
						}
						printf("%c", Detail->NumInt[a]);
					}
				}
				else {
					printf("%c", Detail->NumInt[a]);
				}
				nowLen++, a++, skip++;
			}
			if (nowLen == ScinotationLen)	//有效位输出足够之后,剩下的位忽略,只计算长度
			{
				while (Detail->NumInt[a] != 0)
					a++, skip++;
			}

			a = 0;
			while (nowLen < ScinotationLen&&Detail->NumFloat[a] != 0)
			{
				if (!ReserveZero)	//不保留多余的0
				{
					if (Detail->NumFloat[a] == '0')
						unPrintZero += 1;
					else
					{
						if (unPrintZero != 0)
						{
							printf("%0*d", unPrintZero, 0);
							unPrintZero = 0;
						}
						printf("%c", Detail->NumFloat[a]);
					}
				}
				else printf("%c", Detail->NumFloat[a]);

				nowLen++, a++;
			}
			printf("E+%d\n", skip);
		}
	}
	else {
		//直接输出数字
		printf("%s", Detail->NumInt);
		unPrintZero = 0;
		if (Detail->NumFloat[0] != 0)
		{
			printf(".");
			if (!ReserveZero) {
				//不保留末尾的0
				while (Detail->NumFloat[a] != 0) {
					if (Detail->NumFloat[a] == '0')
						unPrintZero += 1;
					else
					{
						if (unPrintZero != 0)
						{
							printf("%0*d", unPrintZero, 0);
							unPrintZero = 0;
						}
						printf("%c", Detail->NumFloat[a]);
					}
					a++;
				}
			}
			else {
				printf("%s", Detail->NumFloat);
			}
		}
		printf("\n");
	}
}

//打印该对象的详细信息
/*
##未完成
*/
void BigFigure::printDetail()
{
	printf(
		"数字字符串分配内存:%d(%d+%d)\n"
		"整数部分有效位数:%d\n"
		"小数最大存储精度:%d\n"
		"以科学计数法输出:%s\n"
		"科学计数法输出时有效位数:%d\n"
		"值:",
		Detail->AllocatedSize, Detail->IntAllocatedLen + 1, Detail->AllocatedSize - Detail->IntAllocatedLen - 1,
		Detail->IntLen,
		Detail->Accuracy,
		ScinotationShow ? "是" : "否",
		ScinotationLen
	);
	printfBF();
	return;
}
#endif


/******************************************************************************************
重载函数
*******************************************************************************************/


NumStringDetail::NumStringDetail(std::string NumString) :
	RadixMinus(0), ExpMinus(0), IntBeZero(0), Mode(0), IntLen(0), FloatLen(0), ExpLen(0), IntStart_p(0), FloatStart_p(0), ExpStart_p(0)
{
	this->NumString = NumString;
	NumCheck(*this);
}
/*
template <class T>
std::string FormatToString(const T &Num)
{
	//std::istringstream istr;
	//istr.str(Num);
	//std::ostringstream oss;
	//oss << Num;
	//return istr.str(Num);
	return "23";
}
*/



/******************************************************************************************
基础函数
*******************************************************************************************/
//检查字符串是否为数字,并且判断字符串表示的是整数还是小数
/*
返回值代表的含义如下:
  1.正整数
  -1.负整数
  2.正小数
  -2.负小数
  30.正科学计数数(正指数)
  31.正科学计数数(负指数)
  -30.负科学计数数(正指数)
  -31.负科学计数数(负指数)
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
					if (HasPoint)
					{
						//小数点部分,设置小数点后字符串的指针
						//NumDetail.FloatStart_p = index_p;
					}
					else if (IntBeZero)
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
				else NumDetail.ExpMinus = 1;
			}
			else {
				//处理底数部分
				if (NumDetail.RadixMinus)
				{
					//在底数部分已有负号时(存在两个负号),数字无效
					NumDetail.Mode = 0;
					return 0;
				}
				else NumDetail.RadixMinus = 1;
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


	//如果是正确的数字, 则可以通过这个for的验证
	//NumDetail.ExpMinus = Expminus;
	//NumDetail.RadixMinus = Radixminus;
	NumDetail.IntBeZero = IntBeZero;
	if (Scinotation)
	{
		NumDetail.Mode = 2;		//初始化为2,Mode将为3,4
		NumDetail.ExpLen = index_p - NumDetail.ExpStart_p;
	}
	else
	{
		NumDetail.Mode = 0;		//初始化为0,Mode将为1,2
		NumDetail.FloatLen = index_p - NumDetail.FloatStart_p;
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
		NumDetail.IntLen = index_p - NumDetail.IntStart_p;
	}

	return 1;
}
#if 0
//计算一个已知类型的字符串的整数部分和小数部分的长度,并通过result返回
/*
result[0]表示的是整数的位数
result[1]表示的是浮点数的位数
返回值0 失败
返回值1 成功(整数)
返回值2 成功(小数)

有严重bug,待完整测试

*/
int BitCount(std::string NumString, int NumType, int result[2])
{
	const char* tempString = NumString.c_str();
	size_t len, len2;
	bool hasNumPre = false;			//记录在指定符号前方是否有数字
	int exponent;			//用于科学计数法的计算位数,保存科学计数法的指数部分
	switch (NumType)
	{
	case 1:		//正整数
		result[0] = NumString.length();
		return 1;
	case -1:	//负整数
		result[0] = NumString.length() - 1;
		return 1;
	case 2:		//正小数
		len = 0;
		while (tempString[len] != '.')len++;
		result[0] = len;
		result[1] = NumString.length() - len - 1;
		return 2;
	case -2:	//负小数
		len = 0;
		while (tempString[len] != '.')len++;
		result[0] = len - 1;
		result[1] = NumString.length() - len - 2;
		return 2;
	case 30:	//正小数(正指数)
		len2 = 0;
		while (tempString[len2] != 'E' && tempString[len2] != 'e')len2++;
		exponent = atoi(tempString + len2 + 1);

		len = 0;
		while (tempString[len] != '.'&&len < len2)len++;		//找到小数点所在的位置
		result[0] = len + exponent;
		if (len == len2)
			result[1] = len2 - len - exponent;
		else
			result[1] = len2 - len - 1 - exponent;				//多减去一个小数点
		if (result[1] < 0)
		{
			result[1] = 0;
			return 1;
		}
		else
			return 2;
	case 31:	//正小数(负指数)
		len2 = 0;
		while (tempString[len2] != 'E' && tempString[len2] != 'e')len2++;
		exponent = atoi(tempString + len2 + 2);
		len = 0;
		while (tempString[len] != '.'&&len < len2)len++;		//找到小数点所在的位置
		result[0] = len - exponent;
		if (len == len2)
			result[1] = len2 - len + exponent;
		else
			result[1] = len2 - len - 1 + exponent;				//多减去一个小数点
		if (result[0] < 1)
			result[0] = 1;
		return 2;
	case -30:	//负小数(正指数)
		/*
		缺少支持-0.01E2这种情况
		*/
		len2 = 0;
		while (tempString[len2] != 'E' && tempString[len2] != 'e') len2++;
		exponent = atoi(tempString + len2 + 1);
		len = 1;
		hasNumPre = false;
		while (tempString[len] != '.'&&len < len2)
		{
			if (!hasNumPre&&tempString[len] != '0')
				hasNumPre = true;
			len++;			//找到小数点所在的位置
		}
		if (!hasNumPre)
		{
			while (tempString[len] != '0'&&len < len2)len--;
			len--;
		}
		result[0] = len - 1 + exponent;								//多减去一个负号

		if (len == len2)
			result[1] = len2 - len - 1 - exponent;
		else
			result[1] = len2 - len - 2 - exponent;					//多减去一个小数点
		if (result[1] < 0)
		{
			result[1] = 0;
			return 1;
		}
		else
		{
			return 2;
		}
	case -31:	//负小数(负指数)
		len2 = 0;
		while (tempString[len2] != 'E' && tempString[len2] != 'e')len2++;
		exponent = atoi(tempString + len2 + 1)*-1;

		len = 0;
		while (tempString[len] != '.'&&len < len2)len++;		//找到小数点所在的位置
		result[0] = len - 1 - exponent;							//多减去一个负号
		if (len == len2)
			result[1] = len2 - len + exponent;
		else
			result[1] = len2 - len - 1 + exponent;				//多减去一个小数点
		if (result[0] < 1)
			result[0] = 1;
		return 2;
	case 0:
	default:
		return 0;
	}
}

//BitCount的简化参数版本(自动检查数字字符串的合法性)
/*
函数说明参见BitCount()
*/
int BitCount_check(std::string NumString, int result[2])
{
	return BitCount(NumString, NumCheck(NumString), result);
}

#endif

std::string FormatToString(double Num)
{
	return std::string();
}

/*
template<class T>

void to_string(string & result, const T& t)

{

	ostringstream oss;//创建一个流

	oss << t;//把值传递如流中

	result = oss.str();//获取转换后的字符转并将其写入result
}
*/
