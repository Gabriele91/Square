//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include <Square/Square.h>
#include <iostream>
int main()
{
	using namespace Square;
	JsonValue value
	({ 
		{"nome", "mario" },
		{"eta",  10.0    },
	});
    
    for(auto it : value.object())
    {
        if (it.second.is_string())
        {
            std::cout << it.second.string() << std::endl;
        }
        else if (it.second.is_number())
        {
            std::cout << it.second.is_number() << std::endl;
        }
    }
    
    JSon json(R"JSON(
               {
                   "mario" : 1.2e-3,
                   "v5" : [1,2,3,5],
                   "m2x2" : [ [1,2], [3,4] ]
               }
           )JSON");
    
    
    if(json.errors().size())
        std::cout << json.errors() << std::endl;
    else
    {
        std::cout << "mario: " << json.document()["mario"].number() << std::endl;
        for(int i=0;i!=json.document()["v5"].size(); ++i) std::cout << "v5: " << json.document()["v5"][i].number() << std::endl;
        
        for(int x=0;x!=json.document()["m2x2"].size(); ++x)
            for(int y=0;y!=json.document()["m2x2"][x].size(); ++y)
                std::cout << "m2x2: " << json.document()["m2x2"][x][y].number() << std::endl;
    }

    return 0;
}
