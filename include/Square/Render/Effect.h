//
//  Effect.h
//  Square
//
//  Created by Gabriele Di Bari on 09/04/18.
//  Copyright � 2018 Gabriele Di Bari. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Core/Resource.h"
#include "Square/Driver/Render.h"
#include "Square/Render/Queue.h"
#include "Square/Render/VertexLayout.h"

namespace Square
{
namespace Resource
{
	class Texture;
	class Shader;
}
}

namespace Square
{
namespace Render
{
	//declaretion
	class Effect;
	class EffectPass;
	class EffectTechnique;
	struct EffectQueueType;
	struct EffectParameter;
	
	//parameter type
	enum class EffectParameterType
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

	//static reflaction
	template < class T, typename Enable = void >
	inline typename std::enable_if< !std::is_convertible< T, std::shared_ptr<ResourceObject> >::value, EffectParameterType >::type parameter_type_traits()
	{
		return EffectParameterType::PT_NONE;
	};
	template < class T >
	inline typename std::enable_if< std::is_convertible< T, std::shared_ptr<ResourceObject> >::value, EffectParameterType >::type parameter_type_traits()
	{
		return EffectParameterType::PT_RESOURCE;
	};
	//template specialization
	template <> inline EffectParameterType parameter_type_traits< Shared<Resource::Texture> >() { return EffectParameterType::PT_TEXTURE; };
	template <> inline EffectParameterType parameter_type_traits<int>() { return EffectParameterType::PT_INT; };
	template <> inline EffectParameterType parameter_type_traits<float>() { return EffectParameterType::PT_FLOAT; };
	template <> inline EffectParameterType parameter_type_traits<double>() { return EffectParameterType::PT_DOUBLE; };

	template <> inline EffectParameterType parameter_type_traits<IVec2>() { return EffectParameterType::PT_VEC2; };
	template <> inline EffectParameterType parameter_type_traits<IVec3>() { return EffectParameterType::PT_VEC3; };
	template <> inline EffectParameterType parameter_type_traits<IVec4>() { return EffectParameterType::PT_VEC4; };

	template <> inline EffectParameterType parameter_type_traits<Vec2>() { return EffectParameterType::PT_VEC2; };
	template <> inline EffectParameterType parameter_type_traits<Vec3>() { return EffectParameterType::PT_VEC3; };
	template <> inline EffectParameterType parameter_type_traits<Vec4>() { return EffectParameterType::PT_VEC4; };
	template <> inline EffectParameterType parameter_type_traits<Mat3>() { return EffectParameterType::PT_MAT3; };
	template <> inline EffectParameterType parameter_type_traits<Mat4>() { return EffectParameterType::PT_MAT4; };

	template <> inline EffectParameterType parameter_type_traits<DVec2>() { return EffectParameterType::PT_DVEC2; };
	template <> inline EffectParameterType parameter_type_traits<DVec3>() { return EffectParameterType::PT_DVEC3; };
	template <> inline EffectParameterType parameter_type_traits<DVec4>() { return EffectParameterType::PT_DVEC4; };
	template <> inline EffectParameterType parameter_type_traits<DMat3>() { return EffectParameterType::PT_DMAT3; };
	template <> inline EffectParameterType parameter_type_traits<DMat4>() { return EffectParameterType::PT_DMAT4; };

	template <> inline EffectParameterType parameter_type_traits< std::vector<Shared<Resource::Texture> > >() { return EffectParameterType::PT_STD_VECTOR_TEXTURE; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<int> >() { return EffectParameterType::PT_STD_VECTOR_INT; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<float> >() { return EffectParameterType::PT_STD_VECTOR_FLOAT; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<double> >() { return EffectParameterType::PT_STD_VECTOR_DOUBLE; };

	template <> inline EffectParameterType parameter_type_traits< std::vector<IVec2> >() { return EffectParameterType::PT_STD_VECTOR_VEC2; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<IVec3> >() { return EffectParameterType::PT_STD_VECTOR_VEC3; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<IVec4> >() { return EffectParameterType::PT_STD_VECTOR_VEC4; };

	template <> inline EffectParameterType parameter_type_traits< std::vector<Vec2> >() { return EffectParameterType::PT_STD_VECTOR_VEC2; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<Vec3> >() { return EffectParameterType::PT_STD_VECTOR_VEC3; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<Vec4> >() { return EffectParameterType::PT_STD_VECTOR_VEC4; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<Mat3> >() { return EffectParameterType::PT_STD_VECTOR_MAT3; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<Mat4> >() { return EffectParameterType::PT_STD_VECTOR_MAT4; };

	template <> inline EffectParameterType parameter_type_traits< std::vector<DVec2> >() { return EffectParameterType::PT_STD_VECTOR_DVEC2; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<DVec3> >() { return EffectParameterType::PT_STD_VECTOR_DVEC3; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<DVec4> >() { return EffectParameterType::PT_STD_VECTOR_DVEC4; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<DMat3> >() { return EffectParameterType::PT_STD_VECTOR_DMAT3; };
	template <> inline EffectParameterType parameter_type_traits< std::vector<DMat4> >() { return EffectParameterType::PT_STD_VECTOR_DMAT4; };

	//definitions
	struct SQUARE_API EffectQueueType
	{
		QueueType m_type{ Render::QueueType::RQ_OPAQUE };
		int       m_order{ 0 };
	};

	//parameter class
	struct SQUARE_API EffectParameter
	{
		virtual void set(Shared< Resource::Texture > texture);
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

		virtual void set(const std::vector < Shared< Resource::Texture > >& i);
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

		//init
		EffectParameter(Allocator* allocator);
		virtual ~EffectParameter(); 

		//build a paramter
		static Unique<EffectParameter> create(Allocator* allocator, EffectParameterType type);

		template < class T >
		static Unique<EffectParameter> create() { return create(parameter_type_traits<T>()); }

		template < class T >
		static Unique<EffectParameter> create(Allocator* allocator, const T& value)
		{
			auto param = create(allocator, parameter_type_traits<T>());
			param->set(value);
			return std::move(param);
		}

		//is valid
		virtual bool is_valid();

		//type
		EffectParameterType get_type();


		virtual Shared<Resource::Texture> get_texture()  const;
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

		virtual const std::vector< Shared<Resource::Texture> >&  get_vector_texture()  const;
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

		virtual Unique<EffectParameter> copy() const = 0;
		virtual void copy_to(EffectParameter*) const = 0;
		virtual void* value_ptr() = 0;
		template < class T > T* value_ptr() { return (T*)value_ptr(); }

	protected:

		friend class		Effect;
		int				    m_id{ -1 };
		EffectParameterType m_type{ EffectParameterType::PT_NONE };
		Allocator*			m_allocator{ nullptr };

	};

	//alias vector of parameters
	using EffectParameters = std::vector < Unique< EffectParameter > >;
	
	//input pass //todo: rename in pipeline inputs
	struct SQUARE_API EffectPassInputs
	{
		//geometry/render
		Render::ConstBuffer*   m_camera{ nullptr };
		Render::ConstBuffer*   m_transform{ nullptr };
		//lights
		Vec4				   m_ambient_light{ 1,1,1,1 };
		Render::ConstBuffer*   m_direction_light{ nullptr };
		Render::ConstBuffer*   m_point_light{ nullptr };
		Render::ConstBuffer*   m_spot_light{ nullptr };

		Render::Texture*       m_shadow_map{ nullptr };
		Render::ConstBuffer*   m_direction_shadow_light{ nullptr };
		Render::ConstBuffer*   m_point_shadow_light{ nullptr };
		Render::ConstBuffer*   m_spot_shadow_light{ nullptr };

		EffectPassInputs() = default;

		EffectPassInputs
		(
			//render
			  Render::ConstBuffer*   camera
			, Render::ConstBuffer*   transform
			//lights
			, Vec4				     ambient_light
			, Render::ConstBuffer*   direction_light = nullptr
			, Render::ConstBuffer*   point_light = nullptr
			, Render::ConstBuffer*   spot_light = nullptr

			, Render::Texture*       shadow_map = nullptr
			, Render::ConstBuffer*   direction_shadow_light = nullptr
			, Render::ConstBuffer*   point_shadow_light = nullptr
			, Render::ConstBuffer*   spot_shadow_light = nullptr
		)
		{
			m_camera = camera;
			m_transform = transform;

			m_ambient_light = ambient_light;
			m_direction_light = direction_light;
			m_point_light = point_light;
			m_spot_light = spot_light;

			m_shadow_map = shadow_map;
			m_direction_shadow_light = direction_shadow_light;
			m_point_shadow_light = point_shadow_light;
			m_spot_shadow_light = spot_shadow_light;
		}
	};

	//pass type //todo: rename in pipeline
	class SQUARE_API EffectPass
	{
	public:
		Render::CullfaceState              m_cullface;
		Render::DepthBufferState           m_depth;
		Render::BlendState                 m_blend;
		Shared<Resource::Shader>           m_shader;
		std::vector< int >                 m_param_id;
		std::vector< Render::Uniform* >    m_uniform;
		//default uniform
		Render::UniformConstBuffer*        m_uniform_camera{ nullptr };
		Render::UniformConstBuffer*        m_uniform_transform{ nullptr };
		//all light uniform
		enum LightsType : unsigned int
		{
			LT_NONE      = 0b000000,
			LT_AMBIENT   = 0b000010,
			LT_SPOT      = 0b000100,
			LT_POINT     = 0b001000,
			LT_DIRECTION = 0b010000,
			LT_AREA      = 0b100000,
		};
		LightsType m_support_light { LT_NONE };
		LightsType m_support_shadow{ LT_NONE };
		//uniforms lights
		Render::Uniform*             m_uniform_ambient_light{ nullptr };
		Render::UniformConstBuffer*  m_uniform_spot{ nullptr };
		Render::UniformConstBuffer*  m_uniform_point{ nullptr };
		Render::UniformConstBuffer*  m_uniform_direction{ nullptr };
		//uniforms shadows
		Render::Uniform*			 m_uniform_shadow_map{ nullptr };
		Render::UniformConstBuffer*  m_uniform_spot_shadow{ nullptr };
		Render::UniformConstBuffer*  m_uniform_point_shadow{ nullptr };
		Render::UniformConstBuffer*  m_uniform_direction_shadow{ nullptr };

		//constructor
		EffectPass();
		virtual ~EffectPass();
		
		//unsafe
		void bind(  Render::Context&        render
                  , const EffectPassInputs&	inputs = EffectPassInputs()
                  , EffectParameters*       params = nullptr
                  ) const;

		void bind(  Render::Context& render
                  , EffectParameters* params = nullptr
                  ) const;

		void unbind() const;
	};

	//pass list
	class SQUARE_API EffectTechnique
	{

	public:
		using PastList = std::vector < EffectPass >;
		using PassListIt = PastList::iterator;
		using PassListCIt = PastList::const_iterator;

		//info
		size_t size();

		//alloc
		void resize(size_t n);
		void reserve(size_t n);

		//push
		void push_back(const EffectPass&& pass);
		//operator
		EffectPass& operator[](size_t i);
		const EffectPass& operator[](size_t i) const;

		//iterators
		PassListIt begin();
		PassListIt end();

		PassListCIt begin() const;
		PassListCIt end() const;
		//last one
		EffectPass& back();
		const EffectPass& back() const;

	protected:

		PastList m_passes;

	};

	//alias map of parameters and techniques
	using EffectParametersMap = std::unordered_map< std::string, int >;
	using EffectTechniquesMap = std::unordered_map< std::string, EffectTechnique >;

	//Effect
	class SQUARE_API Effect : public BaseObject
	{
	public:
		//Init object
		SQUARE_OBJECT(Effect)

		//contructor
		Effect(Allocator* allocator);
		virtual ~Effect();

		//no copy
		Effect(const Effect&) = delete;
		Effect& operator = (const Effect&) = delete;

		//no move
		Effect(const Effect&&) = delete;
		Effect& operator=(const Effect&&) = delete;


		//set queue
		void  queue(const EffectQueueType& queue) { m_queue = queue; }
		const EffectQueueType& queue() const { return m_queue; }

		//get technique
		EffectTechnique* technique(const std::string& technique);
		const EffectTechnique* technique(const std::string& technique) const;

		//all techniques
		const EffectTechniquesMap& techniques() const { return m_techniques_map; }

		//get parameter
		EffectParameters*   parameters();
		EffectParameter*    parameter(int parameter) const;
		EffectParameter*    parameter(const std::string& parameter) const;
		const std::string&  parameter_name(int parameter) const;
		Unique<EffectParameters>  copy_all_parameters() const;;

		//get id
		int parameter_id(const std::string& parameter);

		//get technique from other effect
		bool import_technique(const Effect& effect, const std::string& name);

		//get all techniques from other effect
		bool import_techniques(const Effect& effect);

	protected:
		//info
		EffectQueueType		 m_queue;
		EffectParameters     m_parameters;
		EffectParametersMap  m_parameters_map;
		EffectTechniquesMap  m_techniques_map;
		Allocator*			 m_allocator;

		//get id
		int add_parameter(int id, const std::string& name, Unique<EffectParameter>&& parameter);

		//help
		void for_each_pass_build_params_id();
		void for_each_pass_build_params_id(EffectTechnique& effect);

	};

}
}
