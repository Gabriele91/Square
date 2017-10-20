//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include <Square/Square.h>
#include <iostream>
#include <sstream>
int main()
{
    using namespace Square;
    using namespace Square::Data;
	//define
	Json jin
	({
	   //json 
		{
			{"nome", "mario" },
			{"eta",  10.0    },
		}
	  //end
	});
    //serialize + parsing/deserialize
	Json jout(((std::stringstream&)(std::stringstream() << jin)).str());
    //serialize again 
    std::cout << (jout.errors().size() ? jout.errors() : jout.document()) << std::endl;
	//others
	auto obj = 
	R"(
		//sss [[,],{,}]
		[{"m":[2,3]} /*??*/,[1,2]]
	)"_json;
	obj[0] = "[1,2,4]"_json;
	std::cout << obj;
	//end
    return 0;
}
