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

    namespace AuxSerializeJson
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
                json_value["dtype"] = "BOOL";
                json_value["value"] = value.get<bool>();
                break;

            case Square::VR_CHAR:
                json_value["dtype"] = "CHAR";
                json_value["value"] = value.get<char>();
                break;

            case Square::VR_SHORT:
                json_value["dtype"] = "SHORT";
                json_value["value"] = value.get<short>();
                break;

            case Square::VR_INT:
                json_value["dtype"] = "INT";
                json_value["value"] = value.get<int>();
                break;

            case Square::VR_LONG:
                json_value["dtype"] = "LONG";
                json_value["value"] = value.get<long>();
                break;

            case Square::VR_LONGLONG:
                json_value["dtype"] = "LONGLONG";
                json_value["value"] = static_cast<long>(value.get<long long>());
                break;

            case Square::VR_UCHAR:
                json_value["dtype"] = "UCHAR";
                json_value["value"] = value.get<unsigned char>();
                break;

            case Square::VR_USHORT:
                json_value["dtype"] = "USHORT";
                json_value["value"] = value.get<unsigned short>();
                break;

            case Square::VR_UINT:
                json_value["dtype"] = "UINT";
                json_value["value"] = value.get<unsigned int>();
                break;

            case Square::VR_ULONG:
                json_value["dtype"] = "ULONG";
                json_value["value"] = static_cast<unsigned int>(value.get<unsigned long>());
                break;

            case Square::VR_ULONGLONG:
                json_value["dtype"] = "ULONGLONG";
                json_value["value"] = static_cast<unsigned int>(value.get<unsigned long long>());
                break;

            case Square::VR_FLOAT:
                json_value["dtype"] = "FLOAT";
                json_value["value"] = value.get<float>();
                break;

            case Square::VR_DOUBLE:
                json_value["dtype"] = "DOUBLE";
                json_value["value"] = value.get<double>();
                break;

            case Square::VR_LONG_DOUBLE:
                json_value["dtype"] = "LONG_DOUBLE";
                json_value["value"] = static_cast<double>(value.get<long double>());
                break;

            case Square::VR_VEC2:
                json_value["dtype"] = "VEC2";
                json_value["value"] = serialize_vec2(value.get<Vec2>());
                break;

            case Square::VR_VEC3:
                json_value["dtype"] = "VEC3";
                json_value["value"] = serialize_vec3(value.get<Vec3>());
                break;

            case Square::VR_VEC4:
                json_value["dtype"] = "VEC4";
                json_value["value"] = serialize_vec4(value.get<Vec4>());
                break;

            case Square::VR_IVEC2:
                json_value["dtype"] = "IVEC2";
                json_value["value"] = serialize_vec2(value.get<IVec2>());
                break;

            case Square::VR_IVEC3:
                json_value["dtype"] = "IVEC3";
                json_value["value"] = serialize_vec3(value.get<IVec3>());
                break;

            case Square::VR_IVEC4:
                json_value["dtype"] = "IVEC4";
                json_value["value"] = serialize_vec4(value.get<IVec4>());
                break;

            case Square::VR_DVEC2:
                json_value["dtype"] = "DVEC2";
                json_value["value"] = serialize_vec2(value.get<DVec2>());
                break;

            case Square::VR_DVEC3:
                json_value["dtype"] = "DVEC3";
                json_value["value"] = serialize_vec3(value.get<DVec3>());
                break;

            case Square::VR_DVEC4:
                json_value["dtype"] = "DVEC4";
                json_value["value"] = serialize_vec4(value.get<DVec4>());
                break;

            case Square::VR_QUAT:
                json_value["dtype"] = "QUAT";
                json_value["value"] = serialize_quad(value.get<Quat>());
                break;

            case Square::VR_DQUAT:
                json_value["dtype"] = "DQUAT";
                json_value["value"] = serialize_quad(value.get<DQuat>());
                break;

            case Square::VR_FLOAT_MATRIX:
                json_value["dtype"] = "FLOAT_MATRIX";
                json_value["value"] = serialize_mat4(value.get<Mat4>());
                break;

            case Square::VR_DOUBLE_MATRIX:
                json_value["dtype"] = "DOUBLE_MATRIX";
                json_value["value"] = serialize_mat4(value.get<DMat4>());
                break;

            case Square::VR_STD_VECTOR_SHORT:
                json_value["dtype"] = "STD_VECTOR_SHORT";
                json_value["value"] = serialize_vector(value.get<std::vector<short>>());
                break;

            case Square::VR_STD_VECTOR_INT:
                json_value["dtype"] = "STD_VECTOR_INT";
                json_value["value"] = serialize_vector(value.get<std::vector<int>>());
                break;

            case Square::VR_STD_VECTOR_LONG:
                json_value["dtype"] = "STD_VECTOR_LONG";
                json_value["value"] = serialize_vector(value.get<std::vector<long>>());
                break;

            case Square::VR_STD_VECTOR_LONGLONG:
                json_value["dtype"] = "STD_VECTOR_LONGLONG";
                json_value["value"] = serialize_vector_w_cast<long long, long>(value.get<std::vector<long long>>());
                break;

            case Square::VR_STD_VECTOR_USHORT:
                json_value["dtype"] = "STD_VECTOR_USHORT";
                json_value["value"] = serialize_vector(value.get<std::vector<unsigned short>>());
                break;

            case Square::VR_STD_VECTOR_UINT:
                json_value["dtype"] = "STD_VECTOR_UINT";
                json_value["value"] = serialize_vector(value.get<std::vector<unsigned int>>());
                break;

            case Square::VR_STD_VECTOR_ULONG:
                json_value["dtype"] = "STD_VECTOR_ULONG";
                json_value["value"] = serialize_vector_w_cast<unsigned long, unsigned int>(value.get<std::vector<unsigned long>>());
                break;

            case Square::VR_STD_VECTOR_ULONGLONG:
                json_value["dtype"] = "STD_VECTOR_ULONGLONG";
                json_value["value"] = serialize_vector_w_cast<unsigned long long, unsigned int>(value.get<std::vector<unsigned long long>>());
                break;

            case Square::VR_STD_VECTOR_FLOAT:
                json_value["dtype"] = "STD_VECTOR_FLOAT";
                json_value["value"] = serialize_vector(value.get<std::vector<float>>());
                break;

            case Square::VR_STD_VECTOR_DOUBLE:
                json_value["dtype"] = "STD_VECTOR_DOUBLE";
                json_value["value"] = serialize_vector(value.get<std::vector<double>>());
                break;

            case Square::VR_STD_VECTOR_LONG_DOUBLE:
                json_value["dtype"] = "STD_VECTOR_LONG_DOUBLE";
                json_value["value"] = serialize_vector_w_cast<long double, double>(value.get<std::vector<long double>>());
                break;

            case Square::VR_STD_VECTOR_VEC2:
                json_value["dtype"] = "STD_VECTOR_VEC2";
                json_value["value"] = serialize_vector(value.get<std::vector<Vec2>>());
                break;

            case Square::VR_STD_VECTOR_VEC3:
                json_value["dtype"] = "STD_VECTOR_VEC3";
                json_value["value"] = serialize_vector(value.get<std::vector<Vec3>>());
                break;

            case Square::VR_STD_VECTOR_VEC4:
                json_value["dtype"] = "STD_VECTOR_VEC4";
                json_value["value"] = serialize_vector(value.get<std::vector<Vec4>>());
                break;

            case Square::VR_STD_VECTOR_IVEC2:
                json_value["dtype"] = "STD_VECTOR_IVEC2";
                json_value["value"] = serialize_vector(value.get<std::vector<IVec2>>());
                break;

            case Square::VR_STD_VECTOR_IVEC3:
                json_value["dtype"] = "STD_VECTOR_IVEC3";
                json_value["value"] = serialize_vector(value.get<std::vector<IVec3>>());
                break;

            case Square::VR_STD_VECTOR_IVEC4:
                json_value["dtype"] = "STD_VECTOR_IVEC4";
                json_value["value"] = serialize_vector(value.get<std::vector<IVec4>>());
                break;

            case Square::VR_STD_VECTOR_DVEC2:
                json_value["dtype"] = "STD_VECTOR_DVEC2";
                json_value["value"] = serialize_vector(value.get<std::vector<DVec2>>());
                break;

            case Square::VR_STD_VECTOR_DVEC3:
                json_value["dtype"] = "STD_VECTOR_DVEC3";
                json_value["value"] = serialize_vector(value.get<std::vector<DVec3>>());
                break;

            case Square::VR_STD_VECTOR_DVEC4:
                json_value["dtype"] = "STD_VECTOR_DVEC4";
                json_value["value"] = serialize_vector(value.get<std::vector<DVec4>>());
                break;

            case Square::VR_STD_VECTOR_FLOAT_MATRIX:
                json_value["dtype"] = "STD_VECTOR_FLOAT_MATRIX";
                json_value["value"] = serialize_vector(value.get<std::vector<Mat4>>());
                break;

            case Square::VR_STD_VECTOR_DOUBLE_MATRIX:
                json_value["dtype"] = "STD_VECTOR_DOUBLE_MATRIX";
                json_value["value"] = serialize_vector(value.get<std::vector<DMat4>>());
                break;

            case Square::VR_STD_STRING:
                json_value["dtype"] = "STD_STRING";
                json_value["value"] = value.get<std::string>();
                break;

            case Square::VR_STD_VECTOR_STRING:
                json_value["dtype"] = "STD_VECTOR_STRING";
                json_value["value"] = serialize_vector(value.get<std::vector<std::string>>());
                break;

            case Square::VR_RESOURCE:
                json_value["dtype"] = "RESOURCE";
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

    namespace AuxDeserializeJson
    {
        template<typename V>
        TVec2<V> deserialize_vec2(const JsonValue& json_vec)
        {
            if (!json_vec.is_object())
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            TVec2<V> vec;
            vec.x = (V)json_vec["x"].number();
            vec.y = (V)json_vec["y"].number();
            return vec;
        }

        template<typename V>
        TVec3<V> deserialize_vec3(const JsonValue& json_vec)
        {
            if (!json_vec.is_object())
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            TVec3<V> vec;
            vec.x = (V)json_vec["x"].number();
            vec.y = (V)json_vec["y"].number();
            vec.z = (V)json_vec["z"].number();
            return vec;
        }

        template<typename V>
        TVec4<V> deserialize_vec4(const JsonValue& json_vec)
        {
            if (!json_vec.is_object())
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            TVec4<V> vec;
            vec.x = (V)json_vec["x"].number();
            vec.y = (V)json_vec["y"].number();
            vec.z = (V)json_vec["z"].number();
            vec.w = (V)json_vec["w"].number();
            return vec;
        }

        template<typename V>
        TQuat<V> deserialize_quad(const JsonValue& json_vec)
        {
            if (!json_vec.is_object())
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            TQuat<V> quad;
            quad.x = (V)json_vec["x"].number();
            quad.y = (V)json_vec["y"].number();
            quad.z = (V)json_vec["z"].number();
            quad.w = (V)json_vec["w"].number();
            return quad;
        }

        template<typename V>
        TMat4<V> deserialize_mat4(const JsonValue& json_vec)
        {
            if (!json_vec.is_array() || json_vec.array().size() < 16)
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            TMat4<V> mat4;
            for (int y = 0; y != 4; ++y)
            for (int x = 0; x != 4; ++x)
            {
                mat4[x][y] = (V)(json_vec[y * 4 + x].number());
            }
            return mat4;
        }


        template<typename T>
        std::vector<T> deserialize_vector_number(const JsonValue& json_vec)
        {
            if (!json_vec.is_array())
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            std::vector<T> vec;
            vec.resize(json_vec.array().size());
            for (size_t i = 0; i < json_vec.array().size(); ++i)
            {
                vec[i] = (T)(json_vec[i].number());
            }
            return vec;
        }

        template<typename V>
        std::vector< TVec2<V> > deserialize_vector_vec2(const JsonValue& json_vec)
        {
            if (!json_vec.is_array())
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            std::vector<  TVec2<V> > vec;
            vec.resize(json_vec.array().size());
            for (size_t i = 0; i < json_vec.array().size(); ++i)
            {
                vec[i] = deserialize_vec2<V>(json_vec[i]);
            }
            return vec;
        }

        template<typename V>
        std::vector< TVec3<V> > deserialize_vector_vec3(const JsonValue& json_vec)
        {
            if (!json_vec.is_array())
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            std::vector<  TVec3<V> > vec;
            vec.resize(json_vec.array().size());
            for (size_t i = 0; i < json_vec.array().size(); ++i)
            {
                vec[i] = deserialize_vec3<V>(json_vec[i]);
            }
            return vec;
        }

        template<typename V>
        std::vector< TVec4<V> > deserialize_vector_vec4(const JsonValue& json_vec)
        {
            if (!json_vec.is_array())
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            std::vector<  TVec4<V> > vec;
            vec.resize(json_vec.array().size());
            for (size_t i = 0; i < json_vec.array().size(); ++i)
            {
                vec[i] = deserialize_vec4<V>(json_vec[i]);
            }
            return vec;
        }

        template<typename V>
        std::vector< TMat4<V> > deserialize_vector_mat4(const JsonValue& json_vec)
        {
            if (!json_vec.is_array())
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            std::vector< TMat4<V> > vec;
            vec.resize(json_vec.array().size());
            for (size_t i = 0; i < json_vec.array().size(); ++i)
            {
                vec[i] = deserialize_mat4<V>(json_vec[i]);
            }
            return vec;
        }

        std::vector< std::string > deserialize_vector_string(const JsonValue& json_vec)
        {
            if (!json_vec.is_array())
            {
                throw std::runtime_error("deserialize_json: Invalid field type");
            }
            std::vector< std::string > vec;
            vec.resize(json_vec.array().size());
            for (size_t i = 0; i < json_vec.array().size(); ++i)
            {
                if (!json_vec[i].is_string())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                vec[i] = json_vec[i].string();
            }
            return vec;
        }

        Variant deserialize_json(const JsonValue& json, Square::Context& context)
        {
            if (!json.is_object())
            {
                throw std::runtime_error("deserialize_json: It is not an object");
            }

            if (!json.contains("dtype") || !json["dtype"].is_string())
            {
                throw std::runtime_error("deserialize_json: Missing 'dtype' field");
            }

            std::string dtype = json["dtype"].string();
            const Data::JsonValue& json_value = json["value"];
            Variant value;

            if (dtype == "BOOL") 
            {
                if(!json_value.is_boolean())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = json_value.boolean();
            }
            else if (dtype == "CHAR") 
            {
                if (!json_value.is_string())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = json_value.string().c_str()[0];
            }
            else if (dtype == "SHORT") 
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<short>(json_value.number());
            }
            else if (dtype == "INT") 
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<int>(json_value.number());
            }
            else if (dtype == "LONG")
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<long>(json_value.number());
            }
            else if (dtype == "LONG_LONG")
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<long long>(json_value.number());
            }
            else if (dtype == "UNSIGNED_CHAR")
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<unsigned char>(json_value.number());
            }
            else if (dtype == "UNSIGNED_SHORT")
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<unsigned short>(json_value.number());
            }
            else if (dtype == "UNSIGNED_INT")
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<unsigned int>(json_value.number());
            }
            else if (dtype == "UNSIGNED_LONG")
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<unsigned long>(json_value.number());
            }
            else if (dtype == "UNSIGNED_LONG_LONG")
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<unsigned long long>(json_value.number());
            }
            else if (dtype == "FLOAT")
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<float>(json_value.number());
            }
            else if (dtype == "DOUBLE")
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<double>(json_value.number());
            }
            else if (dtype == "LONG_DOUBLE")
            {
                if (!json_value.is_number())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = static_cast<long double>(json_value.number());
            }
            else if (dtype == "VEC2") 
            {
                value = deserialize_vec2<float>(json_value);
            }
            else if (dtype == "VEC3") 
            {
                value = deserialize_vec3<float>(json_value);
            }
            else if (dtype == "VEC4") 
            {
                value = deserialize_vec4<float>(json_value);
            }
            else if (dtype == "IVEC2") 
            {
                value = deserialize_vec2<int>(json_value);
            }
            else if (dtype == "IVEC3") 
            {
                value = deserialize_vec3<int>(json_value);
            }
            else if (dtype == "IVEC4") 
            {
                value = deserialize_vec4<int>(json_value);
            }
            else if (dtype == "DVEC2")
            {
                value = deserialize_vec2<double>(json_value);
            }
            else if (dtype == "DVEC3")
            {
                value = deserialize_vec3<double>(json_value);
            }
            else if (dtype == "DVEC4") 
            {
                value = deserialize_vec4<double>(json_value);
            }
            else if (dtype == "QUAT") 
            {
                value = deserialize_quad<float>(json_value);
            }
            else if (dtype == "DQUAT")
            {
                value = deserialize_quad<double>(json_value);
            }
            else if (dtype == "FLOAT_MATRIX")
            {
                value = deserialize_mat4<float>(json_value);
            }
            else if (dtype == "DOUBLE_MATRIX")
            {
                value = deserialize_mat4<double>(json_value);
            }
            else if (dtype == "STD_VECTOR_SHORT")
            {
                value = deserialize_vector_number<short>(json_value);
            }
            else if (dtype == "STD_VECTOR_INT")
            {
                value = deserialize_vector_number<int>(json_value);
            }
            else if (dtype == "STD_VECTOR_LONG")
            {
                value = deserialize_vector_number<long>(json_value);
            }
            else if (dtype == "STD_VECTOR_LONG_LONG")
            {
                value = deserialize_vector_number<long long>(json_value);
            }
            else if (dtype == "STD_VECTOR_UNSIGNED_SHORT")
            {
                value = deserialize_vector_number<unsigned short>(json_value);
            }
            else if (dtype == "STD_VECTOR_UNSIGNED_INT")
            {
                value = deserialize_vector_number<unsigned int>(json_value);
            }
            else if (dtype == "STD_VECTOR_UNSIGNED_LONG")
            {
                value = deserialize_vector_number<unsigned long>(json_value);
            }
            else if (dtype == "STD_VECTOR_UNSIGNED_LONG_LONG")
            {
                value = deserialize_vector_number<unsigned long long>(json_value);
            }
            else if (dtype == "STD_VECTOR_FLOAT")
            {
                value = deserialize_vector_number<float>(json_value);
            }
            else if (dtype == "STD_VECTOR_DOUBLE")
            {
                value = deserialize_vector_number<double>(json_value);
            }
            else if (dtype == "STD_VECTOR_LONG_DOUBLE")
            {
                value = deserialize_vector_number<long double>(json_value);
            }
            else if (dtype == "STD_VECTOR_VEC2")
            {
                value = deserialize_vector_vec2<float>(json_value);
            }
            else if (dtype == "STD_VECTOR_VEC3")
            {
                value = deserialize_vector_vec3<float>(json_value);
            }
            else if (dtype == "STD_VECTOR_VEC4")
            {
                value = deserialize_vector_vec4<float>(json_value);
            }
            else if (dtype == "STD_VECTOR_IVEC2")
            {
                value = deserialize_vector_vec2<int>(json_value);
            }
            else if (dtype == "STD_VECTOR_IVEC3")
            {
                value = deserialize_vector_vec3<int>(json_value);
            }
            else if (dtype == "STD_VECTOR_IVEC4")
            {
                value = deserialize_vector_vec4<int>(json_value);
            }
            else if (dtype == "STD_VECTOR_DVEC2")
            {
                value = deserialize_vector_vec2<double>(json_value);
            }
            else if (dtype == "STD_VECTOR_DVEC3")
            {
                value = deserialize_vector_vec3<double>(json_value);
            }
            else if (dtype == "STD_VECTOR_DVEC4")
            {
                value = deserialize_vector_vec4<double>(json_value);
            }
            else if (dtype == "STD_VECTOR_MAT4")
            {
                value = deserialize_vector_mat4<float>(json_value);
            }
            else if (dtype == "STD_VECTOR_DMAT4")
            {
                value = deserialize_vector_mat4<double>(json_value);
            }
            else if (dtype == "STD_STRING")
            {
                if (!json_value.is_string())
                {
                    throw std::runtime_error("deserialize_json: Invalid field type");
                }
                value = json_value.string();
            }
            else if (dtype == "STD_VECTOR_STRING")
            {
                value = deserialize_vector_string(json_value);
            }
            else if (dtype == "RESOURCE") 
            {
                if (!json_value.contains("exists")
                ||  !json_value["exists"].is_boolean())
                {
                    throw std::runtime_error("deserialize_json: Invalid resource");
                }
                if (json_value["exists"].boolean()) 
                {
                    if (!json_value.contains("resource_name")
                    ||  !json_value["resource_name"].is_string())
                    {
                        throw std::runtime_error("deserialize_json: Invalid resource name");
                    }
                    std::string resource_name = json_value["resource_name"].string();
                    // Recupera la risorsa
                    Shared<ResourceObject> resource = context.resource(resource_name);
                    if (!resource) 
                    {
                        throw std::runtime_error("deserialize_json: Resource not found: " + resource_name);
                    }
                    value = resource;
                }
                else 
                {
                    value = Shared<ResourceObject>(); // No resource
                }
            }
            else {
                throw std::runtime_error("deserialize_json: Unknown type: " + dtype);
            }

            return value;
        }
    }

    SQUARE_API bool attribute_serialize_json(JsonValue& archive, const Object* object, const std::vector < Attribute >* attributes)
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
            archive[attr.name()] = AuxSerializeJson::serialize_json(value);
        }
        return true;
    }
    
    SQUARE_API bool attribute_deserialize_json(JsonValue& archive, Object* object, const std::vector < Attribute >* attributes)
    {
		if (!attributes) return false;
        //deserialize
        for(const Attribute& attr : *attributes)
        if (attr.type() & Attribute::FILE)
        if (archive.contains(attr.name()))
        {
            //Get attribute
            JsonValue& jattribute = archive[attr.name()];
            //Deserialize
            Variant value = AuxDeserializeJson::deserialize_json(jattribute, object->context());
            //set
            attr.set(object, value.as_variant_ref());
        }
        return true;
    }
}
}
