//
//  AttributeSerialize.cpp
//  Square
//
//  Created by Gabriele Di Bari on 14/11/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Square/Data/AttributeSerialize.h"

namespace Square
{
namespace  Data
{
    SQUARE_API bool attribute_serialize(Archive& archive, const Object* object, const std::vector < Attribute >* attributes)
    {
		if (!attributes) return false;
        //serialize
        for(const Attribute& attr : *attributes)
        if (attr.type() & Attribute::Type::FILE)
        {
            //get
            VariantRef value;
            attr.get(object, value);
            //serialize
            archive % value;
        }
        return true;
    }
    
    SQUARE_API bool attribute_deserialize(Archive& archive, Object* object, const std::vector < Attribute >* attributes)
    {
		if (!attributes) return false;
        //deserialize
        for(const Attribute& attr : *attributes)
        if (attr.type() & Attribute::FILE)
        {
            //deserialize
            Variant value(attr.value_type());
            archive % value.as_variant_ref();
            //set
            attr.set(object, value.as_variant_ref());
        }
        return true;
    }

    namespace Aux
    {
        template<typename V>
        Json serialize_vec2(const TVec2<V>& vec)
        {
            JsonObject json_vec;
            json_vec["x"] = vec.x;
            json_vec["y"] = vec.y;
            return json_vec;
        }

        template<typename V>
        Json serialize_vec3(const TVec3<V>& vec)
        {
            JsonObject json_vec;
            json_vec["x"] = vec.x;
            json_vec["y"] = vec.y;
            json_vec["z"] = vec.z;
            return json_vec;
        }

        template<typename V>
        Json serialize_vec4(const TVec4<V>& vec)
        {
            JsonObject json_vec;
            json_vec["x"] = vec.x;
            json_vec["y"] = vec.y;
            json_vec["z"] = vec.z;
            json_vec["w"] = vec.w;
            return json_vec;
        }

        template<typename V>
        Json serialize_quad(const TQuat<V>& vec)
        {
            JsonObject json_vec;
            json_vec["x"] = vec.x;
            json_vec["y"] = vec.y;
            json_vec["z"] = vec.z;
            json_vec["w"] = vec.w;
            return json_vec;
        }

        template<typename V>
        Json serialize_mat4(const TMat4<V>& value)
        {
            JsonArray json_array;
            json_array.resize(16);
            for (int y = 0; y != 4; ++y)
            for (int x = 0; x != 4; ++x)
            {
                json_array[y * 4 + x] = value[x][y];
            }
            return json_array;
        }

        template<typename T, typename C>
        Json serialize_vector_w_cast(const std::vector<T>& vec)
        {
            JsonArray json_array;
            json_array.resize(vec.size());
            for (size_t i = 0; i < vec.size(); ++i)
            {
                json_array[i] = static_cast<C>(vec[i]);
            }
            return json_array;
        }

        template<typename T>
        Json serialize_vector(const std::vector<T>& vec)
        {
            JsonArray json_array;
            json_array.resize(vec.size());
            for (size_t i = 0; i < vec.size(); ++i)
            {
                json_array[i] = vec[i];
            }
            return json_array;
        }

        template<typename V>
        Json serialize_vector(const std::vector< TVec2<V> >& vec)
        {
            JsonArray json_array;
            json_array.resize(vec.size());
            for (size_t i = 0; i < vec.size(); ++i)
            {
                json_array[i] = serialize_vec2(vec[i]);
            }
            return json_array;
        }

        template<typename V>
        Json serialize_vector(const std::vector< TVec3<V> >& vec)
        {
            JsonArray json_array;
            json_array.resize(vec.size());
            for (size_t i = 0; i < vec.size(); ++i)
            {
                json_array[i] = serialize_vec3(vec[i]);
            }
            return json_array;
        }

        template<typename V>
        Json serialize_vector(const std::vector< TVec4<V> >& vec)
        {
            JsonArray json_array;
            json_array.resize(vec.size());
            for (size_t i = 0; i < vec.size(); ++i)
            {
                json_array[i] = serialize_vec4(vec[i]);
            }
            return json_array;
        }

        template<typename V>
        Json serialize_vector(const std::vector< TQuat<V> >& vec)
        {
            JsonArray json_array;
            json_array.resize(vec.size());
            for (size_t i = 0; i < vec.size(); ++i)
            {
                json_array[i] = serialize_quad(vec[i]);
            }
            return json_array;
        }

        template<typename V>
        Json serialize_vector(const std::vector< TMat4<V> >& vec)
        {
            JsonArray json_array;
            json_array.resize(vec.size());
            for (size_t i = 0; i < vec.size(); ++i)
            {
                json_array[i] = serialize_mat4(vec[i]);
            }
            return json_array;
        }

        Json serialize_json(const Variant& value)
        {
            Json json_value = JsonObject();

            switch (value.get_type())
            {
            case Square::VR_NONE: /* none */ break;

            case Square::VR_BOOL:
                json_value["dtype"] = "bool";
                json_value["value"] = value.get<bool>();
                break;

            case Square::VR_CHAR:
                json_value["dtype"] = "char";
                json_value["value"] = value.get<char>();
                break;

            case Square::VR_SHORT:
                json_value["dtype"] = "short";
                json_value["value"] = value.get<short>();
                break;

            case Square::VR_INT:
                json_value["dtype"] = "int";
                json_value["value"] = value.get<int>();
                break;

            case Square::VR_LONG:
                json_value["dtype"] = "long";
                json_value["value"] = value.get<long>();
                break;

            case Square::VR_LONGLONG:
                json_value["dtype"] = "long long";
                json_value["value"] = static_cast<long>(value.get<long long>()); 
                break;

            case Square::VR_UCHAR:
                json_value["dtype"] = "unsigned char";
                json_value["value"] = value.get<unsigned char>();
                break;

            case Square::VR_USHORT:
                json_value["dtype"] = "unsigned short";
                json_value["value"] = value.get<unsigned short>();
                break;

            case Square::VR_UINT:
                json_value["dtype"] = "unsigned int";
                json_value["value"] = value.get<unsigned int>();
                break;

            case Square::VR_ULONG:
                json_value["dtype"] = "unsigned long";
                json_value["value"] = static_cast<unsigned int>(value.get<unsigned long>()); 
                break;

            case Square::VR_ULONGLONG:
                json_value["dtype"] = "unsigned long long";
                json_value["value"] = static_cast<unsigned int>(value.get<unsigned long long>()); 
                break;

            case Square::VR_FLOAT:
                json_value["dtype"] = "float";
                json_value["value"] = value.get<float>();
                break;

            case Square::VR_DOUBLE:
                json_value["dtype"] = "double";
                json_value["value"] = value.get<double>();
                break;

            case Square::VR_LONG_DOUBLE:
                json_value["dtype"] = "long double";
                json_value["value"] = static_cast<double>(value.get<long double>()); 
                break;

            case Square::VR_VEC2:
                json_value["dtype"] = "Vec2";
                json_value["value"] = serialize_vec2(value.get<Vec2>());
                break;

            case Square::VR_VEC3:
                json_value["dtype"] = "Vec3";
                json_value["value"] = serialize_vec3(value.get<Vec3>());
                break;

            case Square::VR_VEC4:
                json_value["dtype"] = "Vec4";
                json_value["value"] = serialize_vec4(value.get<Vec4>());
                break;

            case Square::VR_IVEC2:
                json_value["dtype"] = "IVec2";
                json_value["value"] = serialize_vec2(value.get<IVec2>());
                break;

            case Square::VR_IVEC3:
                json_value["dtype"] = "IVec3";
                json_value["value"] = serialize_vec3(value.get<IVec3>()); 
                break;

            case Square::VR_IVEC4:
                json_value["dtype"] = "IVec4";
                json_value["value"] = serialize_vec4(value.get<IVec4>()); 
                break;

            case Square::VR_DVEC2:
                json_value["dtype"] = "DVec2";
                json_value["value"] = serialize_vec2(value.get<DVec2>()); 
                break;

            case Square::VR_DVEC3:
                json_value["dtype"] = "DVec3";
                json_value["value"] = serialize_vec3(value.get<DVec3>()); 
                break;

            case Square::VR_DVEC4:
                json_value["dtype"] = "DVec4";
                json_value["value"] = serialize_vec4(value.get<DVec4>()); 
                break;

            case Square::VR_QUAT:
                json_value["dtype"] = "Quat";
                json_value["value"] = serialize_quad(value.get<Quat>());
                break;

            case Square::VR_DQUAT:
                json_value["dtype"] = "DQuat";
                json_value["value"] = serialize_quad(value.get<DQuat>());
                break;

            case Square::VR_FLOAT_MATRIX:
                json_value["dtype"] = "Mat4";
                json_value["value"] = serialize_mat4(value.get<Mat4>());
                break;

            case Square::VR_DOUBLE_MATRIX:
                json_value["dtype"] = "DMat4";
                json_value["value"] = serialize_mat4(value.get<DMat4>());
                break;

            case Square::VR_STD_VECTOR_SHORT:
                json_value["dtype"] = "std::vector<short>";
                json_value["value"] = serialize_vector(value.get<std::vector<short>>());
                break;

            case Square::VR_STD_VECTOR_INT:
                json_value["dtype"] = "std::vector<int>";
                json_value["value"] = serialize_vector(value.get<std::vector<int>>());
                break;

            case Square::VR_STD_VECTOR_LONG:
                json_value["dtype"] = "std::vector<long>";
                json_value["value"] = serialize_vector(value.get<std::vector<long>>());
                break;

            case Square::VR_STD_VECTOR_LONGLONG:
                json_value["dtype"] = "std::vector<long long>";
                json_value["value"] = serialize_vector_w_cast<long long, long>(value.get<std::vector<long long>>());
                break;

            case Square::VR_STD_VECTOR_USHORT:
                json_value["dtype"] = "std::vector<unsigned short>";
                json_value["value"] = serialize_vector(value.get<std::vector<unsigned short>>());
                break;

            case Square::VR_STD_VECTOR_UINT:
                json_value["dtype"] = "std::vector<unsigned int>";
                json_value["value"] = serialize_vector(value.get<std::vector<unsigned int>>());
                break;

            case Square::VR_STD_VECTOR_ULONG:
                json_value["dtype"] = "std::vector<unsigned long>";
                json_value["value"] = serialize_vector_w_cast<unsigned long, unsigned int>(value.get<std::vector<unsigned long>>());
                break;

            case Square::VR_STD_VECTOR_ULONGLONG:
                json_value["dtype"] = "std::vector<unsigned long long>";
                json_value["value"] = serialize_vector_w_cast<unsigned long long, unsigned int>(value.get<std::vector<unsigned long long>>());
                break;

            case Square::VR_STD_VECTOR_FLOAT:
                json_value["dtype"] = "std::vector<float>";
                json_value["value"] = serialize_vector(value.get<std::vector<float>>());
                break;

            case Square::VR_STD_VECTOR_DOUBLE:
                json_value["dtype"] = "std::vector<double>";
                json_value["value"] = serialize_vector(value.get<std::vector<double>>());
                break;

            case Square::VR_STD_VECTOR_LONG_DOUBLE:
                json_value["dtype"] = "std::vector<long double>";
                json_value["value"] = serialize_vector_w_cast<long double, double>(value.get<std::vector<long double>>());
                break;

            case Square::VR_STD_VECTOR_VEC2:
                json_value["dtype"] = "std::vector<Vec2>";
                json_value["value"] = serialize_vector(value.get<std::vector<Vec2>>());
                break;

            case Square::VR_STD_VECTOR_VEC3:
                json_value["dtype"] = "std::vector<Vec3>";
                json_value["value"] = serialize_vector(value.get<std::vector<Vec3>>());
                break;

            case Square::VR_STD_VECTOR_VEC4:
                json_value["dtype"] = "std::vector<Vec4>";
                json_value["value"] = serialize_vector(value.get<std::vector<Vec4>>());
                break;

            case Square::VR_STD_VECTOR_IVEC2:
                json_value["dtype"] = "std::vector<IVec2>";
                json_value["value"] = serialize_vector(value.get<std::vector<IVec2>>());
                break;

            case Square::VR_STD_VECTOR_IVEC3:
                json_value["dtype"] = "std::vector<IVec3>";
                json_value["value"] = serialize_vector(value.get<std::vector<IVec3>>());
                break;

            case Square::VR_STD_VECTOR_IVEC4:
                json_value["dtype"] = "std::vector<IVec4>";
                json_value["value"] = serialize_vector(value.get<std::vector<IVec4>>());
                break;

            case Square::VR_STD_VECTOR_DVEC2:
                json_value["dtype"] = "std::vector<DVec2>";
                json_value["value"] = serialize_vector(value.get<std::vector<DVec2>>());
                break;

            case Square::VR_STD_VECTOR_DVEC3:
                json_value["dtype"] = "std::vector<DVec3>";
                json_value["value"] = serialize_vector(value.get<std::vector<DVec3>>());
                break;

            case Square::VR_STD_VECTOR_DVEC4:
                json_value["dtype"] = "std::vector<DVec4>";
                json_value["value"] = serialize_vector(value.get<std::vector<DVec4>>());
                break;

            case Square::VR_STD_VECTOR_FLOAT_MATRIX:
                json_value["dtype"] = "std::vector<Mat4>";
                json_value["value"] = serialize_vector(value.get<std::vector<Mat4>>());
                break;

            case Square::VR_STD_VECTOR_DOUBLE_MATRIX:
                json_value["dtype"] = "std::vector<DMat4>";
                json_value["value"] = serialize_vector(value.get<std::vector<DMat4>>());
                break;

            case Square::VR_STD_STRING:
                json_value["dtype"] = "std::string";
                json_value["value"] = value.get<std::string>();
                break;

            case Square::VR_RESOURCE:
                json_value["dtype"] = "Resource";
                if (value.get<Shared<ResourceObject>>())
                {
                    json_value["exists"] = true;
                    json_value["resource_name"] = value.get<Shared<ResourceObject>>()->resource_name();
                }
                else
                {
                    json_value["exists"] = false;
                }
                break;

            default:
                throw std::runtime_error("Unknown type");
            }

            return json_value;
        }
    }

    SQUARE_API bool attribute_serialize_json(Json& archive, const Object* object, const std::vector < Attribute >* attributes)
    {
		if (!attributes) return false;
        //serialize
        for(const Attribute& attr : *attributes)
        if (attr.type() & Attribute::Type::FILE)
        {
            //get
            VariantRef value;
            attr.get(object, value);
            // to json
            archive[attr.name()] = Aux::serialize_json(value);
        }
        return true;
    }
    
    SQUARE_API bool attribute_deserialize_json(Json& archive, Object* object, const std::vector < Attribute >* attributes)
    {
		if (!attributes) return false;
        // todo
        return true;
    }
}
}
