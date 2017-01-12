﻿//
//  main.cpp
//  BigNumber
//
//  Created by 张驰荣 on 2016/12/26.
//  Copyright © 2016年 张驰荣. All rights reserved.
//

#include <iostream>
#include <cstdlib>
#include "BigNumber.h"
/*
void test1(std::string testData)
{
	int re[2] = { 0 };
	BF bf(100, 100);
	bf.atoBF(testData);
	std::cout << "正常显示+不保留0" << std::endl;
	ScinotationShow = false;
	ReserveZero = false;
	ScinotationLen = 2;
	bf.printfBF();
	ScinotationLen = 5;
	bf.printfBF();
	ScinotationLen = 10;
	bf.printfBF();
	std::cout <<std::endl <<"科学计数+不保留0" << std::endl;
	ScinotationShow = true;
	ReserveZero = false;
	ScinotationLen = 2;
	bf.printfBF();
	ScinotationLen = 5;
	bf.printfBF();
	ScinotationLen = 10;
	bf.printfBF();
	ScinotationLen = 15;
	bf.printfBF();

	std::cout << std::endl << "正常显示+保留0" << std::endl;
	ScinotationShow = false;
	ReserveZero = true;
	ScinotationLen = 2;
	bf.printfBF();
	ScinotationLen = 5;
	bf.printfBF();
	ScinotationLen = 10;
	bf.printfBF();

	std::cout << std::endl << "科学计数+保留0" << std::endl;
	ScinotationShow = true;
	ReserveZero = true;
	ScinotationLen = 2;
	bf.printfBF();
	ScinotationLen = 5;
	bf.printfBF();
	ScinotationLen = 10;
	bf.printfBF();
	ScinotationLen = 15;
	bf.printfBF();
}
*/
/*
测试数据
123456789 正整数
10012345678901234567890 正整数
-123456789  负整数
123456789.0123456789 正小数
123456789.00000000000122 正小数
-123456789.9988 负小数

科学计数
223123E20 整正正
23456789E-10 整正负
-2341E10 整负正
-654732E-5 整负负
1.0001E4 小正正
1.889E-4 小正负
-1.666E3  小负正
-1.555E-4 小负负
123456.789E4 小正正
15673.678E-5 小正负
-6568.5556E3 小负正
-76666677.9E-7 小负负
*/

void AA(BF a)
{
	BF B(25, 9);
	B = a;
	std::cout << "AA" << std::endl;
	B.printDetail();
}

int main(int argc, const char * argv[]) {

	//test1("123456789");
	//test1("10012345678901234567890");
	//test1("-123456789");
	//test1("1234567.012345600");
	//test1("123456789.00000000000122");
	//test1("-123456789.9988");


	//test1("223123E20");
	//test1("-6568.55560000E3");
	//test1("-0.01E2");



	/*
	int re[2] = { 0 };
	BF bf(100, 100);
	bf.atoBF("0.001E2");
	ScinotationShow = true;
	ReserveZero = true;
	ScinotationLen = 16;
	bf.printfBF();
	*/
	//std::cout << BitCount_check("1230.1E-4", re)<<std::endl;
	//std::cout << re[0] << "   " << re[1] << std::endl;
	//std::cout << NumCheck("45.789") << std::endl;
	ConfirmWontLossAccuracy = true;
	ConfirmWontLossHighBit = true;
	//ScinotationLen = 11;
	//ScinotationShow = true;

	BF bf(40, 0), bf2(40, 0), bf3(40, 0);
	 bf = "1999";
	bf2 = "10";

	

	core_IntAdd(bf3, bf, (long)9456);
	//std::cout << typeid(int).hash_code() << std::endl;;
	bf3.printDetail();
	/*
	bf2.toBF(NumStringDetail("-12345000000000000000000620.03200020"));
	bf = bf2;
	bf.printDetail();
	bf2.printDetail();
	std::cout << BFCmp(bf, bf2) << std::endl;
	std::cout << bf << std::endl;
	*/
#if 0
	bf.toBF(
		NumStringDetail("-1889001000.333E-2")
	);
#else
	//NumStringDetail n("-1889001000");
#endif
	system("pause");
	return 0;
}
