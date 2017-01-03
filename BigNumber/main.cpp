//
//  main.cpp
//  BigNumber
//
//  Created by 张驰荣 on 2016/12/26.
//  Copyright © 2016年 张驰荣. All rights reserved.
//

#include <iostream>
#include "BigNumber.h"

int main(int argc, const char * argv[]) {
	//BF bf(10, 10);
	//bf.atoBF("456.56");
	//bf.printfBF();
	int re[2] = {0};
	std::cout << BitCount_check("1230E2", re)<<std::endl;
	std::cout << re[0] << "   " << re[1] << std::endl;
	//std::cout << NumCheck("45.789") << std::endl;
	system("pause");
    return 0;
}
