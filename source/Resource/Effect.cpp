//
//  Effect.cpp
//  Square
//
//  Created by Gabriele Di Bari on 09/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#include "Square/Config.h"
#include "Square/Core/Context.h"
#include "Square/Core/Filesystem.h"
#include "Square/Resource/Effect.h"
#include "Square/Data/ParserEffect.h"
#include "Square/Core/ClassObjectRegistration.h"

namespace Square
{
namespace Resource
{
    //////////////////////////////////////////////////////////////
    //Add element to objects
    SQUARE_CLASS_OBJECT_REGISTRATION(Effect);
    //Registration in context
    void Effect::object_registration(Context& ctx)
    {
        //factory
        ctx.add_resource<Effect>({ ".sqfx", ".shfx", ".fx" });
    }
    //////////////////////////////////////////////////////////////

    using  ParametersMap  = Effect::ParametersMap;
    using  TechniquesMap  = Effect::TechniquesMap;
    using  Technique      = Effect::Technique;
    using  Parameter      = Effect::Parameter;
    using  Parameters     = Effect::Parameters;
    using  ParameterType  = Effect::ParameterType;
    using  ParameterQueue = Effect::ParameterQueue;
    
    //Parameters
    void Effect::Parameter::set(Shared<Resource::Texture> texture) {}
    void Effect::Parameter::set(int i) {}
    void Effect::Parameter::set(float f) {}
    void Effect::Parameter::set(double d) {}
    
    void Effect::Parameter::set(const IVec2& v2) {}
    void Effect::Parameter::set(const IVec3& v3) {}
    void Effect::Parameter::set(const IVec4& v4) {}
    
    void Effect::Parameter::set(const Vec2& v2) {}
    void Effect::Parameter::set(const Vec3& v3) {}
    void Effect::Parameter::set(const Vec4& v4) {}
    void Effect::Parameter::set(const Mat3& m3) {}
    void Effect::Parameter::set(const Mat4& m4) {}
    
    void Effect::Parameter::set(const DVec2& v2) {}
    void Effect::Parameter::set(const DVec3& v3) {}
    void Effect::Parameter::set(const DVec4& v4) {}
    void Effect::Parameter::set(const DMat3& m3) {}
    void Effect::Parameter::set(const DMat4& m4) {}
    
    void Effect::Parameter::set(const int* i, size_t n) {}
    void Effect::Parameter::set(const float* f, size_t n) {}
    void Effect::Parameter::set(const double* d, size_t n) {}
    void Effect::Parameter::set(const IVec2* v2, size_t n) {}
    void Effect::Parameter::set(const IVec3* v3, size_t n) {}
    void Effect::Parameter::set(const IVec4* v4, size_t n) {}
    void Effect::Parameter::set(const Vec2* v2, size_t n) {}
    void Effect::Parameter::set(const Vec3* v3, size_t n) {}
    void Effect::Parameter::set(const Vec4* v4, size_t n) {}
    void Effect::Parameter::set(const Mat3* m3, size_t n) {}
    void Effect::Parameter::set(const Mat4* m4, size_t n) {}
    void Effect::Parameter::set(const DVec2* v2, size_t n) {}
    void Effect::Parameter::set(const DVec3* v3, size_t n) {}
    void Effect::Parameter::set(const DVec4* v4, size_t n) {}
    void Effect::Parameter::set(const DMat3* m3, size_t n) {}
    void Effect::Parameter::set(const DMat4* m4, size_t n) {}
    
    void Effect::Parameter::set(const std::vector < Shared< Texture > >& i) {}
    void Effect::Parameter::set(const std::vector < int >& i) {}
    void Effect::Parameter::set(const std::vector < float >& f) {}
    void Effect::Parameter::set(const std::vector < double >& d) {}
    void Effect::Parameter::set(const std::vector < IVec2 >& v2) {}
    void Effect::Parameter::set(const std::vector < IVec3 >& v3) {}
    void Effect::Parameter::set(const std::vector < IVec4 >& v4) {}
    void Effect::Parameter::set(const std::vector < Vec2 >& v2) {}
    void Effect::Parameter::set(const std::vector < Vec3 >& v3) {}
    void Effect::Parameter::set(const std::vector < Vec4 >& v4) {}
    void Effect::Parameter::set(const std::vector < Mat3 >& m3) {}
    void Effect::Parameter::set(const std::vector < Mat4 >& m4) {}
    void Effect::Parameter::set(const std::vector < DVec2 >& v2) {}
    void Effect::Parameter::set(const std::vector < DVec3 >& v3) {}
    void Effect::Parameter::set(const std::vector < DVec4 >& v4) {}
    void Effect::Parameter::set(const std::vector < DMat3 >& m3) {}
    void Effect::Parameter::set(const std::vector < DMat4 >& m4) {}
    
    Effect::Parameter::Parameter() {}
    Effect::Parameter::~Parameter(){}
    
    bool Effect::Parameter::is_valid() { return m_id >= 0; }
    ParameterType Effect::Parameter::get_type() { return m_type; }
    
    template< class T >
    inline static const T& none_const_t_return()
    {
        const static T none;
        return none;
    }
    
    Shared<Texture> Effect::Parameter::get_texture()  const { return Shared<Texture>(nullptr); }
    int             Effect::Parameter::get_int()      const { return 0;    }
    float           Effect::Parameter::get_float()    const { return 0.0f; }
    double          Effect::Parameter::get_double()   const { return 0.0; }
    const IVec2&    Effect::Parameter::get_ivec2()    const { return none_const_t_return<IVec2>();  }
    const IVec3&    Effect::Parameter::get_ivec3()    const { return none_const_t_return<IVec3>();  }
    const IVec4&    Effect::Parameter::get_ivec4()    const { return none_const_t_return<IVec4>();  }
    const Vec2&     Effect::Parameter::get_vec2()     const { return none_const_t_return<Vec2>();  }
    const Vec3&     Effect::Parameter::get_vec3()     const { return none_const_t_return<Vec3>();  }
    const Vec4&     Effect::Parameter::get_vec4()     const { return none_const_t_return<Vec4>();  }
    const Mat3&     Effect::Parameter::get_mat3()     const { return none_const_t_return<Mat3>();  }
    const Mat4&     Effect::Parameter::get_mat4()     const { return none_const_t_return<Mat4>();  }
    const DVec2&    Effect::Parameter::get_dvec2()    const { return none_const_t_return<DVec2>();  }
    const DVec3&    Effect::Parameter::get_dvec3()    const { return none_const_t_return<DVec3>();  }
    const DVec4&    Effect::Parameter::get_dvec4()    const { return none_const_t_return<DVec4>();  }
    const DMat3&    Effect::Parameter::get_dmat3()    const { return none_const_t_return<DMat3>();  }
    const DMat4&    Effect::Parameter::get_dmat4()    const { return none_const_t_return<DMat4>();  }
    
    const std::vector< Shared<Texture> >&  Effect::Parameter::get_vector_texture()  const { return none_const_t_return< std::vector< Shared<Texture> > >(); }
    const std::vector<int>&    Effect::Parameter::get_vector_int()    const { return none_const_t_return< std::vector<int> >(); }
    const std::vector<float>&  Effect::Parameter::get_vector_float()  const { return none_const_t_return< std::vector<float> >(); }
    const std::vector<double>& Effect::Parameter::get_vector_double() const { return none_const_t_return< std::vector<double> >(); }
    
    const std::vector<IVec2>&  Effect::Parameter::get_vector_ivec2()  const { return none_const_t_return< std::vector<IVec2> >(); }
    const std::vector<IVec3>&  Effect::Parameter::get_vector_ivec3()  const { return none_const_t_return< std::vector<IVec3> >(); }
    const std::vector<IVec4>&  Effect::Parameter::get_vector_ivec4()  const { return none_const_t_return< std::vector<IVec4> >(); }
    
    const std::vector<Vec2>&   Effect::Parameter::get_vector_vec2()   const { return none_const_t_return< std::vector<Vec2> >(); }
    const std::vector<Vec3>&   Effect::Parameter::get_vector_vec3()   const { return none_const_t_return< std::vector<Vec3> >(); }
    const std::vector<Vec4>&   Effect::Parameter::get_vector_vec4()   const { return none_const_t_return< std::vector<Vec4> >(); }
    const std::vector<Mat3>&   Effect::Parameter::get_vector_mat3()   const { return none_const_t_return< std::vector<Mat3> >(); }
    const std::vector<Mat4>&   Effect::Parameter::get_vector_mat4()   const { return none_const_t_return< std::vector<Mat4> >(); }
    
    const std::vector<DVec2>&  Effect::Parameter::get_vector_dvec2()  const { return none_const_t_return< std::vector<DVec2> >(); }
    const std::vector<DVec3>&  Effect::Parameter::get_vector_dvec3()  const { return none_const_t_return< std::vector<DVec3> >(); }
    const std::vector<DVec4>&  Effect::Parameter::get_vector_dvec4()  const { return none_const_t_return< std::vector<DVec4> >(); }
    const std::vector<DMat3>&  Effect::Parameter::get_vector_dmat3()  const { return none_const_t_return< std::vector<DMat3> >(); }
    const std::vector<DMat4>&  Effect::Parameter::get_vector_dmat4()  const { return none_const_t_return< std::vector<DMat4> >(); }

    //struct by type
    struct ParameterTexture : public Parameter
    {
        //value
        Shared<Texture> m_value;
        //init
        ParameterTexture()
        {
            m_value = Shared<Texture>(nullptr);
            m_type  = Effect::PT_TEXTURE;
        }
        ParameterTexture(Shared<Texture> value)
        {
            m_value = value;
            m_type  = Effect::PT_TEXTURE;
        }
        //set value
        virtual void set(Shared<Texture> value) override
        {
            m_value = value;
        }
        //get value
        virtual Shared<Texture> get_texture() const override
        {
            return m_value;
        }
        //copy
        virtual Parameter* copy() const override
        {
            return new ParameterTexture(m_value);
        }
        //ptr
        virtual void* value_ptr() override { return (void*)&m_value; }
    };
    struct ParameterVectorTexture : public Parameter
    {
        //value
        std::vector< Shared<Texture> > m_value;
        //init
        ParameterVectorTexture()
        {
            m_value = {};
            m_type  = Effect::PT_STD_VECTOR_TEXTURE;
        }
        ParameterVectorTexture(const std::vector< Shared<Texture> >& value)
        {
            m_value = value;
            m_type  = Effect::PT_STD_VECTOR_TEXTURE;
        }
        //set value
        virtual void set(const std::vector< Shared<Texture> >& value) override
        {
            m_value = value;
        }
        //get value
        virtual const std::vector< Shared<Texture> >& get_vector_texture() const override
        {
            return m_value;
        }
        //copy
        virtual Parameter* copy() const override
        {
            return new ParameterVectorTexture(m_value);
        }
        //ptr
        virtual void* value_ptr() override { return (void*)&m_value; }
    };
    //////////////////////////////////////////////////////////////////////////
    //utility
    #define DefineValueParameter(Classname,Type,defval)\
    struct Classname : public Parameter\
    {\
        Type m_value;\
        Classname(Type value = defval)\
        {m_value = value; m_type = parameter_type_traits<Type>();}\
        virtual void set(Type value) override { m_value = value; }\
        virtual Type get_ ## Type () const override { return m_value; }\
        virtual Parameter* copy() const  override {  return new Classname(m_value); }\
        virtual void* value_ptr() override { return (void*)&m_value; }\
    }
    #define DefineObjectParameter(Classname,Type, getname, defval)\
    struct Classname : public Parameter\
    {\
        Type m_value;\
        Classname(Type value = defval)\
        {m_value = value; m_type = parameter_type_traits<Type>();}\
        virtual void set(const Type& value) override { m_value = value; }\
        virtual const Type& get_ ## getname () const override { return m_value; }\
        virtual Parameter* copy() const override {  return new Classname(m_value); }\
        virtual void* value_ptr() override { return (void*)&m_value; }\
    }
    #define DefineVectorObjectParameter(Classname,Type, getname)\
    struct Classname : public Parameter\
    {\
        std::vector< Type > m_value;\
        Classname(const std::vector< Type >& value = {})\
        {m_value = value; m_type = parameter_type_traits<Type>();}\
        virtual void set(const std::vector< Type >& value) override { m_value = value; }\
        virtual const std::vector< Type >& get_vector_ ## getname () const override { return m_value; }\
        virtual Parameter* copy() const override {  return new Classname(m_value); }\
        virtual void* value_ptr() override { return (void*)&m_value; }\
    }
    //////////////////////////////////////////////////////////////////////////
    DefineValueParameter(ParameterInt, int, 0);
    DefineValueParameter(ParameterFloat, float, 0.0f);
    DefineValueParameter(ParameterDouble, double, 0.0);
    
    DefineObjectParameter(ParameterIVec2, IVec2, ivec2, IVec2(0));
    DefineObjectParameter(ParameterIVec3, IVec3, ivec3, IVec3(0));
    DefineObjectParameter(ParameterIVec4, IVec4, ivec4, IVec4(0));
    
    DefineObjectParameter(ParameterVec2, Vec2, vec2, Vec2(0));
    DefineObjectParameter(ParameterVec3, Vec3, vec3, Vec3(0));
    DefineObjectParameter(ParameterVec4, Vec4, vec4, Vec4(0));
    DefineObjectParameter(ParameterMat3, Mat3, mat3, Mat3(1));
    DefineObjectParameter(ParameterMat4, Mat4, mat4, Mat4(1));
    
    DefineObjectParameter(ParameterDVec2, DVec2, dvec2, DVec2(0));
    DefineObjectParameter(ParameterDVec3, DVec3, dvec3, DVec3(0));
    DefineObjectParameter(ParameterDVec4, DVec4, dvec4, DVec4(0));
    DefineObjectParameter(ParameterDMat3, DMat3, dmat3, DMat3(1));
    DefineObjectParameter(ParameterDMat4, DMat4, dmat4, DMat4(1));
    
    DefineVectorObjectParameter(ParameterVectorInt, int, int);
    DefineVectorObjectParameter(ParameterVectorFloat, float, float);
    DefineVectorObjectParameter(ParameterVectorDouble, double, double);
    
    DefineVectorObjectParameter(ParameterVectorIVec2, IVec2, ivec2);
    DefineVectorObjectParameter(ParameterVectorIVec3, IVec3, ivec3);
    DefineVectorObjectParameter(ParameterVectorIVec4, IVec4, ivec4);
    
    DefineVectorObjectParameter(ParameterVectorVec2, Vec2, vec2);
    DefineVectorObjectParameter(ParameterVectorVec3, Vec3, vec3);
    DefineVectorObjectParameter(ParameterVectorVec4, Vec4, vec4);
    DefineVectorObjectParameter(ParameterVectorMat3, Mat3, mat3);
    DefineVectorObjectParameter(ParameterVectorMat4, Mat4, mat4);
    
    DefineVectorObjectParameter(ParameterVectorDVec2, DVec2, dvec2);
    DefineVectorObjectParameter(ParameterVectorDVec3, DVec3, dvec3);
    DefineVectorObjectParameter(ParameterVectorDVec4, DVec4, dvec4);
    DefineVectorObjectParameter(ParameterVectorDMat3, DMat3, dmat3);
    DefineVectorObjectParameter(ParameterVectorDMat4, DMat4, dmat4);
    //////////////////////////////////////////////////////////////////////////
    Parameter* Effect::create_parameter(Effect::ParameterType type)
    {
        switch (type)
        {
            case Effect::PT_TEXTURE:return new ParameterTexture();
            case Effect::PT_INT:    return new ParameterInt();
            case Effect::PT_FLOAT:  return new ParameterFloat();
            case Effect::PT_DOUBLE: return new ParameterDouble();
                
            case Effect::PT_IVEC2: return new ParameterIVec2();
            case Effect::PT_IVEC3: return new ParameterIVec3();
            case Effect::PT_IVEC4: return new ParameterIVec4();
                
            case Effect::PT_VEC2: return new ParameterVec2();
            case Effect::PT_VEC3: return new ParameterVec3();
            case Effect::PT_VEC4: return new ParameterVec4();
            case Effect::PT_MAT3: return new ParameterMat3();
            case Effect::PT_MAT4: return new ParameterMat4();
                
            case Effect::PT_DVEC2: return new ParameterDVec2();
            case Effect::PT_DVEC3: return new ParameterDVec3();
            case Effect::PT_DVEC4: return new ParameterDVec4();
            case Effect::PT_DMAT3: return new ParameterDMat3();
            case Effect::PT_DMAT4: return new ParameterDMat4();

            case Effect::PT_STD_VECTOR_TEXTURE:return new ParameterVectorTexture();
            case Effect::PT_STD_VECTOR_INT:    return new ParameterVectorInt();
            case Effect::PT_STD_VECTOR_FLOAT:  return new ParameterVectorFloat();
            case Effect::PT_STD_VECTOR_DOUBLE: return new ParameterVectorDouble();
                
            case Effect::PT_STD_VECTOR_IVEC2: return new ParameterVectorIVec2();
            case Effect::PT_STD_VECTOR_IVEC3: return new ParameterVectorIVec3();
            case Effect::PT_STD_VECTOR_IVEC4: return new ParameterVectorIVec4();
                
            case Effect::PT_STD_VECTOR_VEC2: return new ParameterVectorVec2();
            case Effect::PT_STD_VECTOR_VEC3: return new ParameterVectorVec3();
            case Effect::PT_STD_VECTOR_VEC4: return new ParameterVectorVec4();
            case Effect::PT_STD_VECTOR_MAT3: return new ParameterVectorMat3();
            case Effect::PT_STD_VECTOR_MAT4: return new ParameterVectorMat4();
                
            case Effect::PT_STD_VECTOR_DVEC2: return new ParameterVectorDVec2();
            case Effect::PT_STD_VECTOR_DVEC3: return new ParameterVectorDVec3();
            case Effect::PT_STD_VECTOR_DVEC4: return new ParameterVectorDVec4();
            case Effect::PT_STD_VECTOR_DMAT3: return new ParameterVectorDMat3();
            case Effect::PT_STD_VECTOR_DMAT4: return new ParameterVectorDMat4();
            default: return nullptr; break;
        }
    }
    //enable pass effect
    void Effect::Pass::bind(Render::ConstBuffer* camera_cb, Render::ConstBuffer* transform_cb, Parameters* params) const
    {
        //bind
        bind(params);
        //default uniforms
        if (m_uniform_camera && m_uniform_camera->is_valid())
        {
            m_uniform_camera->bind(camera_cb);
        }
        if (m_uniform_transform && m_uniform_transform->is_valid())
        {
            m_uniform_transform->bind(transform_cb);
        }
    }
    
    void Effect::Pass::bind(Parameters* params) const
    {
        //test
        if(!m_effect) return;
        if(!m_effect->context().render()) return;
        //context
        auto& context = m_effect->context();
        auto* render  = context.render();
        //bind
        render->set_blend_state(m_blend);
        render->set_cullface_state(m_cullface);
        render->set_depth_buffer_state(m_depth);
        //bind shader
        m_shader->bind();
        //if null use default
        if (!params) params = &m_effect->m_parameters;
        //uniform
        for (size_t i = 0; i != m_uniform.size(); ++i)
        {
            auto& param = (*params)[m_param_id[i]];
            //uniform value
            switch (param->get_type())
            {
                default:
                case PT_NONE:
                    /* void */
                break;
                //uniform
                case PT_TEXTURE:   m_uniform[i]->set(param->get_texture()->get_context_texture()); break;
                case PT_INT:       m_uniform[i]->set(param->get_int()); break;
                case PT_FLOAT:     m_uniform[i]->set(param->get_float()); break;
                case PT_DOUBLE:    m_uniform[i]->set(param->get_double()); break;
                    
                case PT_IVEC2:     m_uniform[i]->set(param->get_ivec2()); break;
                case PT_IVEC3:     m_uniform[i]->set(param->get_ivec3()); break;
                case PT_IVEC4:     m_uniform[i]->set(param->get_ivec4()); break;
                    
                case PT_VEC2:      m_uniform[i]->set(param->get_vec2()); break;
                case PT_VEC3:      m_uniform[i]->set(param->get_vec3()); break;
                case PT_VEC4:      m_uniform[i]->set(param->get_vec4()); break;
                case PT_MAT3:      m_uniform[i]->set(param->get_mat3()); break;
                case PT_MAT4:      m_uniform[i]->set(param->get_mat4()); break;
                    
                case PT_DVEC2:      m_uniform[i]->set(param->get_dvec2()); break;
                case PT_DVEC3:      m_uniform[i]->set(param->get_dvec3()); break;
                case PT_DVEC4:      m_uniform[i]->set(param->get_dvec4()); break;
                case PT_DMAT3:      m_uniform[i]->set(param->get_dmat3()); break;
                case PT_DMAT4:      m_uniform[i]->set(param->get_dmat4()); break;
                
                #if 0
                case PT_STD_VECTOR_TEXTURE:    m_uniform[i]->set(param->get_vector_texture()/* todo, cast */); break;
                #endif
                    
                case PT_STD_VECTOR_INT:    m_uniform[i]->set(param->get_vector_int()); break;
                case PT_STD_VECTOR_FLOAT:  m_uniform[i]->set(param->get_vector_float()); break;
                case PT_STD_VECTOR_DOUBLE: m_uniform[i]->set(param->get_vector_double()); break;
                    
                case PT_STD_VECTOR_IVEC2: m_uniform[i]->set(param->get_vector_ivec2()); break;
                case PT_STD_VECTOR_IVEC3: m_uniform[i]->set(param->get_vector_ivec3()); break;
                case PT_STD_VECTOR_IVEC4: m_uniform[i]->set(param->get_vector_ivec4()); break;
                    
                case PT_STD_VECTOR_VEC2: m_uniform[i]->set(param->get_vector_vec2()); break;
                case PT_STD_VECTOR_VEC3: m_uniform[i]->set(param->get_vector_vec3()); break;
                case PT_STD_VECTOR_VEC4: m_uniform[i]->set(param->get_vector_vec4()); break;
                case PT_STD_VECTOR_MAT3: m_uniform[i]->set(param->get_vector_mat3()); break;
                case PT_STD_VECTOR_MAT4: m_uniform[i]->set(param->get_vector_mat4()); break;
                    
                case PT_STD_VECTOR_DVEC2: m_uniform[i]->set(param->get_vector_dvec2()); break;
                case PT_STD_VECTOR_DVEC3: m_uniform[i]->set(param->get_vector_dvec3()); break;
                case PT_STD_VECTOR_DVEC4: m_uniform[i]->set(param->get_vector_dvec4()); break;
                case PT_STD_VECTOR_DMAT3: m_uniform[i]->set(param->get_vector_dmat3()); break;
                case PT_STD_VECTOR_DMAT4: m_uniform[i]->set(param->get_vector_dmat4()); break;
            }
        }
    }
    
    //disable pass effect
    void Effect::Pass::unbind() const 
    {
        m_shader->unbind();
    }
    
    //safe enable pass effect
    Render::State Effect::Pass::safe_bind(Render::ConstBuffer* camera_cb, Render::ConstBuffer* transform_cb, Parameters* params) const
    {
        //test
        if(!m_effect) return Render::State();
        if(!m_effect->context().render()) return Render::State();
        //context
        auto& context = m_effect->context();
        auto* render  = context.render();
        //do
        Render::State state = render->get_render_state();
        bind(camera_cb,transform_cb, params);
        return state;
    }
    
    Render::State Effect::Pass::safe_bind(Parameters* params) const
    {
        //test
        if(!m_effect) return Render::State();
        if(!m_effect->context().render()) return Render::State();
        //context
        auto& context = m_effect->context();
        auto* render  = context.render();
        //do
        Render::State state = render->get_render_state();
        bind(params);
        return state;
    }
    //safe disable pass effect
    void Effect::Pass::safe_unbind(const Render::State& state)
    {
        //test
        if(!m_effect) return;
        if(!m_effect->context().render()) return;
        //set state
        m_effect->context().render()->set_render_state(state);
        //unbind
        unbind();
    }
    
    enum shader_define_rendering
    {
        DEF_RENDERING_COLOR,
        DEF_RENDERING_AMBIENT_LIGHT,
        DEF_RENDERING_DIRECTION_LIGHT,
        DEF_RENDERING_POINT_LIGHT,
        DEF_RENDERING_SPOT_LIGHT
    };
    
    const std::string shader_define_table[]=
    {
        "RENDERING_COLOR",
        "RENDERING_AMBIENT_LIGHT",
        "RENDERING_DIRECTION_LIGHT",
        "RENDERING_POINT_LIGHT",
        "RENDERING_SPOT_LIGHT"
    };
    
    //constructor
    Effect::Effect(Context& context):ResourceObject(context) {}
    Effect::Effect(Context& context, const std::string& path):ResourceObject(context){ load(path); }
    
    //load effect
    bool Effect::load(const std::string& path)
    {
        //parser
        Parser::Effect          e_parser;
        Parser::Effect::Context e_context;
        //do parsing
        if(!e_parser.parse(e_context, Filesystem::text_file_read_all(path)))
        {
            context().add_wrong("Effect: " +  path + "\n" + e_context.errors_to_string());
            return false;
        }
        //alloc params
        m_parameters.resize(e_context.m_parameters.size());
        //add params
        for (size_t i = 0; i != m_parameters.size(); ++i)
        {
            //copy
            switch (e_context.m_parameters[i].m_type)
            {
                case PT_TEXTURE:
                    m_parameters[i] = std::unique_ptr< Parameter >( new ParameterTexture( context().resource<Texture>( e_context.m_parameters[i].m_resource[0] ) ));
                break;
                case PT_STD_VECTOR_TEXTURE:
                {
                    //vector of texture
                    std::vector< Shared<Texture> > v_textures;
                    //get all texture
                    for(const std::string& tex_name : e_context.m_parameters[i].m_resource)
                        v_textures.push_back( context().resource<Texture>(tex_name) );
                    //add
                    m_parameters[i] = std::unique_ptr< Parameter >( new ParameterVectorTexture( v_textures ) );
                }
                break;
                default:
                    m_parameters[i] = std::unique_ptr< Parameter > ( e_context.m_parameters[i].m_paramter->copy() );
                break;
            }
            //save id
            m_parameters[i]->m_id = (int)i;
            //add into map
            m_parameters_map[e_context.m_parameters[i].m_name] = (int)i;
        }
        //ref to sub effect
        Parser::Effect::SubEffectField* ptr_sub_effect = nullptr;
        //test sub effect
        for(Parser::Effect::SubEffectField& sub_effect : e_context.m_sub_effect )
        {
            if(sub_effect.m_requirement.test(context().render()))
            {
                ptr_sub_effect = &sub_effect;
                break;
            }
        }
        //fail
        if(!ptr_sub_effect)
        {
            context().add_wrong("Effect: " + path +", All sub effects unsupported.");
            return false;
        }
        //set queue
        set_queue(ptr_sub_effect->m_queue);
        //n_pass
        size_t n_techniques_parser = ptr_sub_effect->m_techniques.size();
        //add tech
        for (size_t t = 0; t != n_techniques_parser; ++t)
        {
            //add into map
            Technique& this_technique = m_techniques_map[ptr_sub_effect->m_techniques[t].m_name];
            //n pass
            size_t n_pass_parser = ptr_sub_effect->m_techniques[t].m_pass.size();
            //alloc pass
            this_technique.reserve(n_pass_parser);
            //add pass
            for (size_t p = 0; p != n_pass_parser; ++p)
            {
                //ref
                Parser::Effect::PassField& parser_pass = ptr_sub_effect->m_techniques[t].m_pass[p];
                //lights sub pass
                int light_sub_pass = parser_pass.m_lights;
                //Type render
                shader_define_rendering current_shader_def = DEF_RENDERING_COLOR;
                //pass
                while(light_sub_pass)
                {
                    //sub light
                    if(light_sub_pass & Parser::Effect::LF_BASE)
                    {
                        current_shader_def = DEF_RENDERING_COLOR;
                        light_sub_pass    ^= Parser::Effect::LF_BASE;
                    }
                    else if(light_sub_pass & Parser::Effect::LF_AMBIENT)
                    {
                        current_shader_def = DEF_RENDERING_AMBIENT_LIGHT;
                        light_sub_pass    ^= Parser::Effect::LF_AMBIENT;
                    }
                    else if(light_sub_pass & Parser::Effect::LF_DIRECTION)
                    {
                        current_shader_def = DEF_RENDERING_DIRECTION_LIGHT;
                        light_sub_pass    ^= Parser::Effect::LF_DIRECTION;
                    }
                    else if(light_sub_pass & Parser::Effect::LF_POINT)
                    {
                        current_shader_def = DEF_RENDERING_POINT_LIGHT;
                        light_sub_pass    ^= Parser::Effect::LF_POINT;
                    }
                    else if(light_sub_pass & Parser::Effect::LF_SPOT)
                    {
                        current_shader_def = DEF_RENDERING_SPOT_LIGHT;
                        light_sub_pass    ^= Parser::Effect::LF_SPOT;
                    }
                    //add pass
                    this_technique.push_back(Effect::Pass());
                    //pass
                    Effect::Pass& this_pass = this_technique[this_technique.size()-1];
                    //get all values
                    this_pass.m_effect   = this;
                    this_pass.m_blend    = parser_pass.m_blend;
                    this_pass.m_cullface = parser_pass.m_cullface;
                    this_pass.m_depth    = parser_pass.m_depth;
                    //shader
                    if (parser_pass.m_shader.m_name)
                    {
                        this_pass.m_shader = context().resource<Shader>(parser_pass.m_shader.m_text);
                    }
                    else
                    {
                        Shader::PreprocessMap shader_defines
                        {
                            { "version", std::to_string(ptr_sub_effect->m_requirement.m_shader_version) },
                            { "define" , shader_define_table[current_shader_def]                        }
                        };
                        //shader
                        this_pass.m_shader = Shared<Shader>(new Shader(context()));
                        //load effect
                        if (!this_pass.m_shader->load(path,
                                                      parser_pass.m_shader.m_text,
                                                      shader_defines,
                                                      parser_pass.m_shader.m_line - 1
                                                      ))
                        {
                            //preproc, debug
                            std::string debug_preproc;
                            for(const Shader::PreprocessElement& preproc : shader_defines)
                            {
                                debug_preproc += "#"+std::get<0>(preproc)+" "+std::get<1>(preproc)+"\n";
                            }
                            //output
                            context().add_wrong("Effect: "
                                                + path
                                                + "\n"
                                                + "Error from technique: " + ptr_sub_effect->m_techniques[t].m_name + ", pass[" + std::to_string(p) + "] ");
                            return false;
                        }
                    }
                    //default uniform
                    this_pass.m_uniform_camera    = this_pass.m_shader->constant_buffer("Camera");
                    this_pass.m_uniform_transform = this_pass.m_shader->constant_buffer("Transform");
                    //default true
                    this_pass.m_support_light = true;
                    //lights uniforms
                    switch (current_shader_def)
                    {
                        case DEF_RENDERING_SPOT_LIGHT:
                            this_pass.m_uniform_spot = this_pass.m_shader->constant_buffer("SpotLight");
                            break;
                        case DEF_RENDERING_POINT_LIGHT:
                            this_pass.m_uniform_point = this_pass.m_shader->constant_buffer("PointLight");
                            break;
                        case DEF_RENDERING_DIRECTION_LIGHT:
                            this_pass.m_uniform_direction = this_pass.m_shader->constant_buffer("DirectionLight");
                            break;
                        case DEF_RENDERING_AMBIENT_LIGHT:
                            this_pass.m_uniform_ambient_light = this_pass.m_shader->uniform("AmbientLight");
                            break;
                        default:
                            this_pass.m_support_light = false;
                            break;
                    }
                    //get uniforms
                    for (auto it : m_parameters_map)
                    {
                        //get
                        Render::Uniform* u_shader = this_pass.m_shader->uniform(it.first.c_str());
                        //test
                        if (u_shader)
                        {
                            //push
                            this_pass.m_param_id.push_back(it.second);
                            this_pass.m_uniform.push_back(u_shader);
                        }
                    }
                }
            }
            
        }
        
        return true;
    }
    
    //get technique
    Effect::Technique* Effect::get_technique(const std::string& technique_name)
    {
        auto it_tech = m_techniques_map.find(technique_name);
        if (it_tech != m_techniques_map.end()) return &it_tech->second;
        return nullptr;
    }
    
    //get parameter
    Effect::Parameter* Effect::get_parameter(int parameter_id)
    {
        if(m_parameters.size() > (size_t)parameter_id)
            return m_parameters[parameter_id].get();
        return nullptr;
    }
    
    Effect::Parameter* Effect::get_parameter(const std::string& parameter_name)
    {
        auto it_param = m_parameters_map.find(parameter_name);
        if(it_param != m_parameters_map.end()) return get_parameter(it_param->second);
        return nullptr;
    }
    
    Effect::Parameters* Effect::copy_all_parameters()
    {
        //alloc vector
        auto new_params = new Parameters(m_parameters.size());
        //copy all
        for (size_t i = 0; i != m_parameters.size(); ++i)
        {
            (*new_params)[i] = std::unique_ptr< Parameter >( m_parameters[i]->copy() );
        }
        return new_params;
    }
    
    //get id
    int Effect::get_parameter_id(const std::string& parameter_name)
    {
        auto it_param = m_parameters_map.find(parameter_name);
        if (it_param != m_parameters_map.end()) return it_param->second;
        return -1;
    }
}
}
