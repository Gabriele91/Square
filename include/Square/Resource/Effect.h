//
//  Effect.h
//  Square
//
//  Created by Gabriele Di Bari on 09/04/18.
//  Copyright © 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once

#include "Square/Config.h"
#include "Square/Core/Resource.h"
#include "Square/Driver/Render.h"
#include "Square/Resource/Shader.h"
#include "Square/Resource/Texture.h"
#include "Square/Render/Queue.h"

namespace Square
{
namespace Resource
{
    class SQUARE_API Effect : public ResourceObject
                            , public SharedObject<Effect>
    {
        
    public:
        
        //queue param
        struct SQUARE_API ParameterQueue
        {
            Render::QueueType m_type { Render::QueueType::RQ_OPAQUE };
            int               m_order{ 0                            };
        };
        
        //parameter type
        enum ParameterType
        {
            PT_NONE,
            PT_TEXTURE,
            PT_RESOURCE,
            PT_INT,
            PT_FLOAT,
            PT_DOUBLE,
            PT_IVEC2,
            PT_IVEC3,
            PT_IVEC4,
            PT_VEC2,
            PT_VEC3,
            PT_VEC4,
            PT_MAT3,
            PT_MAT4,
            PT_DVEC2,
            PT_DVEC3,
            PT_DVEC4,
            PT_DMAT3,
            PT_DMAT4,
            PT_STD_VECTOR_TEXTURE,
            PT_STD_VECTOR_INT,
            PT_STD_VECTOR_FLOAT,
            PT_STD_VECTOR_DOUBLE,
            PT_STD_VECTOR_IVEC2,
            PT_STD_VECTOR_IVEC3,
            PT_STD_VECTOR_IVEC4,
            PT_STD_VECTOR_VEC2,
            PT_STD_VECTOR_VEC3,
            PT_STD_VECTOR_VEC4,
            PT_STD_VECTOR_MAT3,
            PT_STD_VECTOR_MAT4,
            PT_STD_VECTOR_DVEC2,
            PT_STD_VECTOR_DVEC3,
            PT_STD_VECTOR_DVEC4,
            PT_STD_VECTOR_DMAT3,
            PT_STD_VECTOR_DMAT4
        };
        
        //parameter class
        struct SQUARE_API Parameter
        {
            virtual void set(Shared<Resource::Texture> texture);
            virtual void set(int i);
            virtual void set(float f);
            virtual void set(double d);

            virtual void set(const IVec2& v2);
            virtual void set(const IVec3& v3);
            virtual void set(const IVec4& v4);
            
            virtual void set(const Vec2& v2);
            virtual void set(const Vec3& v3);
            virtual void set(const Vec4& v4);
            virtual void set(const Mat3& m3);
            virtual void set(const Mat4& m4);
            
            virtual void set(const DVec2& v2);
            virtual void set(const DVec3& v3);
            virtual void set(const DVec4& v4);
            virtual void set(const DMat3& m3);
            virtual void set(const DMat4& m4);

            virtual void set(const int* i, size_t n);
            virtual void set(const float* f, size_t n);
            virtual void set(const double* d, size_t n);
            virtual void set(const IVec2* v2, size_t n);
            virtual void set(const IVec3* v3, size_t n);
            virtual void set(const IVec4* v4, size_t n);
            virtual void set(const Vec2* v2, size_t n);
            virtual void set(const Vec3* v3, size_t n);
            virtual void set(const Vec4* v4, size_t n);
            virtual void set(const Mat3* m3, size_t n);
            virtual void set(const Mat4* m4, size_t n);
            virtual void set(const DVec2* v2, size_t n);
            virtual void set(const DVec3* v3, size_t n);
            virtual void set(const DVec4* v4, size_t n);
            virtual void set(const DMat3* m3, size_t n);
            virtual void set(const DMat4* m4, size_t n);

            virtual void set(const std::vector < Shared< Texture > >& i);
            virtual void set(const std::vector < int >& i);
            virtual void set(const std::vector < float >& f);
            virtual void set(const std::vector < double >& d);
            virtual void set(const std::vector < IVec2 >& v2);
            virtual void set(const std::vector < IVec3 >& v3);
            virtual void set(const std::vector < IVec4 >& v4);
            virtual void set(const std::vector < Vec2 >& v2);
            virtual void set(const std::vector < Vec3 >& v3);
            virtual void set(const std::vector < Vec4 >& v4);
            virtual void set(const std::vector < Mat3 >& m3);
            virtual void set(const std::vector < Mat4 >& m4);
            virtual void set(const std::vector < DVec2 >& v2);
            virtual void set(const std::vector < DVec3 >& v3);
            virtual void set(const std::vector < DVec4 >& v4);
            virtual void set(const std::vector < DMat3 >& m3);
            virtual void set(const std::vector < DMat4 >& m4);
            
            Parameter();
            virtual ~Parameter();
            
            virtual bool is_valid();
            ParameterType get_type();
            
            
            virtual Shared<Texture> get_texture()  const;
            virtual int             get_int()      const;
            virtual float           get_float()    const;
            virtual double          get_double()   const;
            virtual const IVec2&    get_ivec2()    const;
            virtual const IVec3&    get_ivec3()    const;
            virtual const IVec4&    get_ivec4()    const;
            virtual const Vec2&     get_vec2()     const;
            virtual const Vec3&     get_vec3()     const;
            virtual const Vec4&     get_vec4()     const;
            virtual const Mat3&     get_mat3()     const;
            virtual const Mat4&     get_mat4()     const;
            virtual const DVec2&    get_dvec2()    const;
            virtual const DVec3&    get_dvec3()    const;
            virtual const DVec4&    get_dvec4()    const;
            virtual const DMat3&    get_dmat3()    const;
            virtual const DMat4&    get_dmat4()    const;

            virtual const std::vector< Shared<Texture> >&  get_vector_texture()  const;
            virtual const std::vector<int>&    get_vector_int()    const;
            virtual const std::vector<float>&  get_vector_float()  const;
            virtual const std::vector<double>& get_vector_double() const;
            
            virtual const std::vector<IVec2>&  get_vector_ivec2()  const;
            virtual const std::vector<IVec3>&  get_vector_ivec3()  const;
            virtual const std::vector<IVec4>&  get_vector_ivec4()  const;
            
            virtual const std::vector<Vec2>&   get_vector_vec2()   const;
            virtual const std::vector<Vec3>&   get_vector_vec3()   const;
            virtual const std::vector<Vec4>&   get_vector_vec4()   const;
            virtual const std::vector<Mat3>&   get_vector_mat3()   const;
            virtual const std::vector<Mat4>&   get_vector_mat4()   const;
            
            virtual const std::vector<DVec2>&  get_vector_dvec2()  const;
            virtual const std::vector<DVec3>&  get_vector_dvec3()  const;
            virtual const std::vector<DVec4>&  get_vector_dvec4()  const;
            virtual const std::vector<DMat3>&  get_vector_dmat3()  const;
            virtual const std::vector<DMat4>&  get_vector_dmat4()  const;

            virtual Parameter* copy() const = 0;
            virtual void* value_ptr() = 0;
            template < class T > T* value_ptr(){ return (T*)value_ptr(); }

        protected:
            
            friend class  Effect;
            int           m_id{ -1 };
            ParameterType m_type{ PT_NONE };
            
        };
        
        //Factory
        static Parameter* create_parameter(Effect::ParameterType type);
        
        //parameters list
        using Parameters = std::vector < std::unique_ptr< Parameter > >;
        
        //pass type
        struct SQUARE_API Pass
        {
            Effect*                            m_effect{ nullptr };
            Render::CullfaceState              m_cullface;
            Render::DepthBufferState           m_depth;
            Render::BlendState                 m_blend;
            Shared<Shader>                     m_shader;
            std::vector< int >                 m_param_id;
            std::vector< Render::Uniform* >    m_uniform;
            //default uniform
            Render::UniformConstBuffer*        m_uniform_camera;
            Render::UniformConstBuffer*        m_uniform_transform;
            //all light uniform
            bool m_support_light{ false };
            //uniforms
            Render::Uniform*             m_uniform_ambient_light{ nullptr };
            Render::UniformConstBuffer*  m_uniform_spot;
            Render::UniformConstBuffer*  m_uniform_point;
            Render::UniformConstBuffer*  m_uniform_direction;
            //unsafe
            void bind(Render::ConstBuffer* camera, Render::ConstBuffer* transform, Parameters* params = nullptr) const;
            
            void bind(Parameters* params = nullptr) const;
            
            void unbind();
            //safe
            Render::State safe_bind(Render::ConstBuffer* camera, Render::ConstBuffer* transform, Parameters* params = nullptr) const;
            
            Render::State safe_bind(Parameters* params = nullptr) const;
            
            void safe_unbind(const Render::State&);
        };
        
        //pass list
        class SQUARE_API Technique
        {
            
        public:
            
            using PastList    = std::vector < Pass >;
            using PassListIt  = PastList::iterator;
            using PassListCIt = PastList::const_iterator;
            
            //info
            size_t size()
            {
                return m_passes.size();
            }
            
            //alloc
            void resize(size_t n)
            {
                m_passes.resize(n);
            }
            
            void reserve(size_t n)
            {
                m_passes.reserve(n);
            }
            
            //push
            void push_back(const Pass&& pass)
            {
                m_passes.push_back(std::move(pass));
            }
            
            //operator
            Pass& operator[](size_t i)
            {
                return m_passes[i];
            }
            
            const Pass& operator[](size_t i) const
            {
                return m_passes[i];
            }
            
            //iterators
            PassListIt begin()
            {
                return m_passes.begin();
            }
            
            PassListIt end()
            {
                return m_passes.end();
            }
            
            PassListCIt begin() const
            {
                return m_passes.begin();
            }
            
            PassListCIt end() const
            {
                return m_passes.end();
            }
            
        protected:
            
            PastList m_passes;
            
        };
        
        //maps
        using ParametersMap = std::unordered_map< std::string, int >;
        using TechniquesMap = std::unordered_map< std::string, Technique >;
        
        //constructor
        Effect(Context& context);
        Effect(Context& context, const std::string& path);
        Effect(const Effect&) = delete;
        Effect& operator=(const Effect&) = delete;
        
        //load effect
        bool load();
        
        //set queue
        void set_queue(const ParameterQueue& queue){ m_queue = queue; }
        
        const ParameterQueue& get_queue() const{ return m_queue; }
        
        //get technique
        Technique* get_technique(const std::string& technique);
        
        //all techniques
        const TechniquesMap& get_techniques() const { return m_techniques_map; }
        
        //get parameter
        Parameter*  get_parameter(int parameter);
        Parameter*  get_parameter(const std::string& parameter);
        Parameters* copy_all_parameters();
        
        //get id
        int get_parameter_id(const std::string& parameter);
        
        
    protected:
        ParameterQueue m_queue;
        Parameters     m_parameters;
        ParametersMap  m_parameters_map;
        TechniquesMap  m_techniques_map;
        
    };
    
    
    //static reflaction
    template < class T, typename Enable = void >
    inline typename std::enable_if< !std::is_convertible< T, std::shared_ptr<ResourceObject> >::value, Effect::ParameterType >::type parameter_type_traits()
    {
        return Effect::PT_NONE;
    };
    template < class T >
    inline typename std::enable_if< std::is_convertible< T, std::shared_ptr<ResourceObject> >::value, Effect::ParameterType >::type parameter_type_traits()
    {
        return Effect::PT_RESOURCE;
    };
    //template specialization
    template <> inline Effect::ParameterType parameter_type_traits< Shared<Texture> >() { return Effect::PT_TEXTURE; };
    template <> inline Effect::ParameterType parameter_type_traits<int>() { return Effect::PT_INT; };
    template <> inline Effect::ParameterType parameter_type_traits<float>() { return Effect::PT_FLOAT; };
    template <> inline Effect::ParameterType parameter_type_traits<double>() { return Effect::PT_DOUBLE; };
    
    template <> inline Effect::ParameterType parameter_type_traits<IVec2>() { return Effect::PT_VEC2; };
    template <> inline Effect::ParameterType parameter_type_traits<IVec3>() { return Effect::PT_VEC3; };
    template <> inline Effect::ParameterType parameter_type_traits<IVec4>() { return Effect::PT_VEC4; };
    
    template <> inline Effect::ParameterType parameter_type_traits<Vec2>() { return Effect::PT_VEC2; };
    template <> inline Effect::ParameterType parameter_type_traits<Vec3>() { return Effect::PT_VEC3; };
    template <> inline Effect::ParameterType parameter_type_traits<Vec4>() { return Effect::PT_VEC4; };
    template <> inline Effect::ParameterType parameter_type_traits<Mat3>() { return Effect::PT_MAT3; };
    template <> inline Effect::ParameterType parameter_type_traits<Mat4>() { return Effect::PT_MAT4; };
    
    template <> inline Effect::ParameterType parameter_type_traits<DVec2>() { return Effect::PT_DVEC2; };
    template <> inline Effect::ParameterType parameter_type_traits<DVec3>() { return Effect::PT_DVEC3; };
    template <> inline Effect::ParameterType parameter_type_traits<DVec4>() { return Effect::PT_DVEC4; };
    template <> inline Effect::ParameterType parameter_type_traits<DMat3>() { return Effect::PT_DMAT3; };
    template <> inline Effect::ParameterType parameter_type_traits<DMat4>() { return Effect::PT_DMAT4; };
    
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<Shared<Texture> > >() { return Effect::PT_STD_VECTOR_TEXTURE; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<int> >() { return Effect::PT_STD_VECTOR_INT; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<float> >() { return Effect::PT_STD_VECTOR_FLOAT; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<double> >() { return Effect::PT_STD_VECTOR_DOUBLE; };
    
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<IVec2> >() { return Effect::PT_STD_VECTOR_VEC2; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<IVec3> >() { return Effect::PT_STD_VECTOR_VEC3; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<IVec4> >() { return Effect::PT_STD_VECTOR_VEC4; };
    
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<Vec2> >() { return Effect::PT_STD_VECTOR_VEC2; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<Vec3> >() { return Effect::PT_STD_VECTOR_VEC3; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<Vec4> >() { return Effect::PT_STD_VECTOR_VEC4; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<Mat3> >() { return Effect::PT_STD_VECTOR_MAT3; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<Mat4> >() { return Effect::PT_STD_VECTOR_MAT4; };
    
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<DVec2> >() { return Effect::PT_STD_VECTOR_DVEC2; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<DVec3> >() { return Effect::PT_STD_VECTOR_DVEC3; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<DVec4> >() { return Effect::PT_STD_VECTOR_DVEC4; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<DMat3> >() { return Effect::PT_STD_VECTOR_DMAT3; };
    template <> inline Effect::ParameterType parameter_type_traits< std::vector<DMat4> >() { return Effect::PT_STD_VECTOR_DMAT4; };

}
}

