//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include <Square/Square.h>
#include <iostream>
#include <sstream>

class Sprite : public Square::Scene::Component
{
public:
    
    Sprite(const std::string& name, size_t id) : Component(name, id)
    {
        
    }
    
    virtual void on_attach(Square::Scene::Actor& entity) override
    {
        
    }
    virtual void on_deattch() override
    {
        
    }
    virtual void on_message(const Square::Scene::Message& msg) override
    {
        
    }
};
REGISTERED_COMPONENT(Sprite, "Sprite")

int main()
{
    using namespace Square;
    using namespace Square::Data;
    using namespace Square::Scene;
	//define
	Json jin (Filesystem::text_file_read_all("scene.json"));
    //serialize + parsing/deserialize
	Json jout(((std::stringstream&)(std::stringstream() << jin)).str());
    //serialize again 
    std::cout << (jout.errors().size() ? jout.errors() : jout.document()) << std::endl;
    //test
    Scene::Actor player;
    player.add(ComponentFactory::create("Sprite"));
    //End
    return 0;
}
