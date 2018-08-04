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
		EffectParameter();
		virtual ~EffectParameter(); 

		//build a paramter
		static EffectParameter* create(EffectParameterType type);

		template < class T >
		static EffectParameter* create() { return create(parameter_type_traits<T>()); }

		template < class T >
		static EffectParameter* create(const T& value)
		{
			EffectParameter* param = create(parameter_type_traits<T>()); 
			param->set(value);
			return param;
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

		virtual EffectParameter* copy() const = 0;
		virtual void copy_to(EffectParameter*) const = 0;
		virtual void* value_ptr() = 0;
		template < class T > T* value_ptr() { return (T*)value_ptr(); }

	protected:

		friend class		Effect;
		int				    m_id{ -1 };
		EffectParameterType m_type{ EffectParameterType::PT_NONE };

	};

	//alias vector of parameters
	using EffectParameters = std::vector < Unique< EffectParameter > >;

	//pass type
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
		LightsType m_support_light{ LT_NONE };
		//uniforms
		Render::Uniform*             m_uniform_ambient_light{ nullptr };
		Render::UniformConstBuffer*  m_uniform_spot{ nullptr };
		Render::UniformConstBuffer*  m_uniform_point{ nullptr };
		Render::UniformConstBuffer*  m_uniform_direction{ nullptr };

		//constructor
		EffectPass();
		virtual ~EffectPass();

		//input pass
		struct SQUARE_API Buffers
		{
			Layout::InputLayoutId  m_layout_id{ ~size_t(0) };
			Render::ConstBuffer*   m_camera{ nullptr };
			Render::ConstBuffer*   m_transform{ nullptr };
			Render::ConstBuffer*   m_direction_light{ nullptr };
			Render::ConstBuffer*   m_point_light{ nullptr };
			Render::ConstBuffer*   m_spot_light{ nullptr };

			Buffers() = default;

			Buffers
			(
			  Layout::InputLayoutId  layout_id
			, Render::ConstBuffer*   camera
			, Render::ConstBuffer*   transform
			, Render::ConstBuffer*   direction_light = nullptr
			, Render::ConstBuffer*   point_light = nullptr
			, Render::ConstBuffer*   spot_light = nullptr
			)
			{
				m_layout_id = layout_id;
				m_camera = camera;
				m_transform = transform;
				m_direction_light = direction_light;
				m_point_light = point_light;
				m_spot_light = spot_light;
			}
		};

		//unsafe
		void bind(  Render::Context*       render
                  , const Vec4&			   ambient_light
                  , const Buffers&         buffers = Buffers()
                  , EffectParameters*      params = nullptr
                  ) const;

		void bind(  Render::Context* render
                  , EffectParameters* params = nullptr
                  ) const;

		void unbind() const;
		//safe
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
		Effect();

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

		//all techniques
		const EffectTechniquesMap& techniques() const { return m_techniques_map; }

		//get parameter
		EffectParameters*  parameters();
		EffectParameter*   parameter(int parameter);
		EffectParameter*   parameter(const std::string& parameter);
		EffectParameters*  copy_all_parameters();

		//get id
		int parameter_id(const std::string& parameter);

	protected:
		EffectQueueType		 m_queue;
		EffectParameters     m_parameters;
		EffectParametersMap  m_parameters_map;
		EffectTechniquesMap  m_techniques_map;
		//get id
		int add_parameter(int id, const std::string& name, EffectParameter* parameter);

	};

}
}
