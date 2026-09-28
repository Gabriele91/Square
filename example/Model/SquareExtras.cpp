//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "SquareExtras.h"
#include <sstream>

namespace SquareExtras
{
    const std::string PREFIX = "square_";

    std::string name(const std::string& key)
    {
        if (key.size() <= PREFIX.size() || key.compare(0, PREFIX.size(), PREFIX) != 0) return {};
        return key.substr(PREFIX.size());
    }

    std::optional<std::string> string(const Square::Data::JsonObject& extras, const std::string& key)
    {
        auto it = extras.find(PREFIX + key);
        if (it == extras.end() || !it->second.is_string()) return std::nullopt;
        return it->second.string();
    }

    double number(const Square::Data::JsonValue& value)
    {
        if (value.is_boolean()) return value.boolean() ? 1.0 : 0.0;
        return value.is_number() ? value.number() : 0.0;
    }

    double component(const Square::Data::JsonValue& value, size_t i)
    {
        if (!value.is_array()) return number(value);
        const auto& values = value.array();
        return i < values.size() ? number(values[i]) : 0.0;
    }

    std::string material_value(const Square::Data::JsonValue& value)
    {
        std::ostringstream text;
        if (value.is_number() || value.is_boolean())
        {
            text << "float(" << number(value) << ")";
        }
        else if (value.is_string())
        {
            text << "texture(\"" << value.string() << "\")";
        }
        else if (value.is_array() && value.array().size() >= 2 && value.array().size() <= 4)
        {
            const auto& values = value.array();
            text << "Vec" << values.size() << "(";
            for (size_t i = 0; i < values.size(); ++i) text << (i ? "," : "") << number(values[i]);
            text << ")";
        }
        return text.str();
    }

    void apply(Square::Object& object, const Square::Data::JsonObject& extras)
    {
        using namespace Square;
        const std::vector<Attribute>* attributes = object.context().attributes(object);
        if (!attributes) return;
        for (const Attribute& attribute : *attributes)
        {
            auto it = extras.find(PREFIX + attribute.name());
            if (it == extras.end()) continue;
            const Data::JsonValue& value = it->second;
            Variant variant;
            switch (attribute.value_type())
            {
            case VR_BOOL:   variant = Variant(number(value) != 0.0); break;
            case VR_INT:    variant = Variant(int(number(value))); break;
            case VR_FLOAT:  variant = Variant(float(number(value))); break;
            case VR_DOUBLE: variant = Variant(number(value)); break;
            case VR_VEC2:   variant = Variant(Vec2(component(value, 0), component(value, 1))); break;
            case VR_VEC3:   variant = Variant(Vec3(component(value, 0), component(value, 1), component(value, 2))); break;
            case VR_VEC4:   variant = Variant(Vec4(component(value, 0), component(value, 1), component(value, 2), component(value, 3))); break;
            case VR_IVEC2:  variant = Variant(IVec2(int(component(value, 0)), int(component(value, 1)))); break;
            case VR_STD_STRING: if (value.is_string()) variant = Variant(value.string()); break;
            default: break;
            }
            if (variant.get_type() == VR_NONE) continue;
            attribute.set(&object, variant.as_variant_ref());
        }
    }
}
