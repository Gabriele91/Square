//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include <Square/Square.h>
#include <optional>

//////////////////////////////////////////////////////////////////////////////////////////
//Custom properties of Blender (glTF "extras") for Square: "square_<name>"
namespace SquareExtras
{
    extern const std::string PREFIX;

    //the name without the prefix, empty if it is not a square property
    std::string name(const std::string& key);

    //the string of an extra ("square_effect"), if there is
    std::optional<std::string> string(const Square::Data::JsonObject& extras, const std::string& key);

    //a number of an extra value (a boolean is 0/1)
    double number(const Square::Data::JsonValue& value);

    //the i-th number of an array (or the number itself: all the components)
    double component(const Square::Data::JsonValue& value, size_t i);

    //the value of a material parameter: a number (or a boolean) is a float, an array of 2/3/4
    //numbers a Vec2/3/4, a string name(...) the value as it is (square_shadow_cull "cullface(off)"),
    //another string a texture; empty if it is none of them
    std::string material_value(const Square::Data::JsonValue& value);

    //the attributes of an object (a light...) from the extras "square_<attribute>": the value
    //is turned into the type of the attribute (a number for an IVec2 fills both components)
    void apply(Square::Object& object, const Square::Data::JsonObject& extras);
}
