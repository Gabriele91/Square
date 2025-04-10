#include "Square/Render/Effect.h"
#include "Square/Resource/Shader.h"
#include "Square/Resource/Texture.h"

namespace Square
{
namespace Render
{
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// EffectParameter
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	void EffectParameter::set(Shared<Resource::Texture> texture) {}
	void EffectParameter::set(int i) {}
	void EffectParameter::set(float f) {}
	void EffectParameter::set(double d) {}

	void EffectParameter::set(const IVec2& v2) {}
	void EffectParameter::set(const IVec3& v3) {}
	void EffectParameter::set(const IVec4& v4) {}

	void EffectParameter::set(const Vec2& v2) {}
	void EffectParameter::set(const Vec3& v3) {}
	void EffectParameter::set(const Vec4& v4) {}
	void EffectParameter::set(const Mat3& m3) {}
	void EffectParameter::set(const Mat4& m4) {}

	void EffectParameter::set(const DVec2& v2) {}
	void EffectParameter::set(const DVec3& v3) {}
	void EffectParameter::set(const DVec4& v4) {}
	void EffectParameter::set(const DMat3& m3) {}
	void EffectParameter::set(const DMat4& m4) {}

	void EffectParameter::set(const int* i, size_t n) {}
	void EffectParameter::set(const float* f, size_t n) {}
	void EffectParameter::set(const double* d, size_t n) {}
	void EffectParameter::set(const IVec2* v2, size_t n) {}
	void EffectParameter::set(const IVec3* v3, size_t n) {}
	void EffectParameter::set(const IVec4* v4, size_t n) {}
	void EffectParameter::set(const Vec2* v2, size_t n) {}
	void EffectParameter::set(const Vec3* v3, size_t n) {}
	void EffectParameter::set(const Vec4* v4, size_t n) {}
	void EffectParameter::set(const Mat3* m3, size_t n) {}
	void EffectParameter::set(const Mat4* m4, size_t n) {}
	void EffectParameter::set(const DVec2* v2, size_t n) {}
	void EffectParameter::set(const DVec3* v3, size_t n) {}
	void EffectParameter::set(const DVec4* v4, size_t n) {}
	void EffectParameter::set(const DMat3* m3, size_t n) {}
	void EffectParameter::set(const DMat4* m4, size_t n) {}

	void EffectParameter::set(const std::vector < Shared < Resource::Texture > > & i) {}
	void EffectParameter::set(const std::vector < int >& i) {}
	void EffectParameter::set(const std::vector < float >& f) {}
	void EffectParameter::set(const std::vector < double >& d) {}
	void EffectParameter::set(const std::vector < IVec2 >& v2) {}
	void EffectParameter::set(const std::vector < IVec3 >& v3) {}
	void EffectParameter::set(const std::vector < IVec4 >& v4) {}
	void EffectParameter::set(const std::vector < Vec2 >& v2) {}
	void EffectParameter::set(const std::vector < Vec3 >& v3) {}
	void EffectParameter::set(const std::vector < Vec4 >& v4) {}
	void EffectParameter::set(const std::vector < Mat3 >& m3) {}
	void EffectParameter::set(const std::vector < Mat4 >& m4) {}
	void EffectParameter::set(const std::vector < DVec2 >& v2) {}
	void EffectParameter::set(const std::vector < DVec3 >& v3) {}
	void EffectParameter::set(const std::vector < DVec4 >& v4) {}
	void EffectParameter::set(const std::vector < DMat3 >& m3) {}
	void EffectParameter::set(const std::vector < DMat4 >& m4) {}

	EffectParameter::EffectParameter(Allocator* allocator) : m_allocator(allocator) {}
	EffectParameter::~EffectParameter() {}

	bool EffectParameter::is_valid() { return m_id >= 0; }
	EffectParameterType EffectParameter::get_type() { return m_type; }

	template< class T >
	inline static const T& none_const_t_return()
	{
		const static T none(0.0f);
		return none;
	}

	Shared<Resource::Texture> EffectParameter::get_texture()  const { return Shared<Resource::Texture>(nullptr); }
	int             EffectParameter::get_int()      const { return 0; }
	float           EffectParameter::get_float()    const { return 0.0f; }
	double          EffectParameter::get_double()   const { return 0.0; }
	const IVec2&    EffectParameter::get_ivec2()    const { return none_const_t_return<IVec2>(); }
	const IVec3&    EffectParameter::get_ivec3()    const { return none_const_t_return<IVec3>(); }
	const IVec4&    EffectParameter::get_ivec4()    const { return none_const_t_return<IVec4>(); }
	const Vec2&     EffectParameter::get_vec2()     const { return none_const_t_return<Vec2>(); }
	const Vec3&     EffectParameter::get_vec3()     const { return none_const_t_return<Vec3>(); }
	const Vec4&     EffectParameter::get_vec4()     const { return none_const_t_return<Vec4>(); }
	const Mat3&     EffectParameter::get_mat3()     const { return none_const_t_return<Mat3>(); }
	const Mat4&     EffectParameter::get_mat4()     const { return none_const_t_return<Mat4>(); }
	const DVec2&    EffectParameter::get_dvec2()    const { return none_const_t_return<DVec2>(); }
	const DVec3&    EffectParameter::get_dvec3()    const { return none_const_t_return<DVec3>(); }
	const DVec4&    EffectParameter::get_dvec4()    const { return none_const_t_return<DVec4>(); }
	const DMat3&    EffectParameter::get_dmat3()    const { return none_const_t_return<DMat3>(); }
	const DMat4&    EffectParameter::get_dmat4()    const { return none_const_t_return<DMat4>(); }

	const std::vector< Shared<Resource::Texture> >&  EffectParameter::get_vector_texture()  const { return none_const_t_return< std::vector< Shared<Resource::Texture> > >(); }
	const std::vector<int>&    EffectParameter::get_vector_int()    const { return none_const_t_return< std::vector<int> >(); }
	const std::vector<float>&  EffectParameter::get_vector_float()  const { return none_const_t_return< std::vector<float> >(); }
	const std::vector<double>& EffectParameter::get_vector_double() const { return none_const_t_return< std::vector<double> >(); }

	const std::vector<IVec2>&  EffectParameter::get_vector_ivec2()  const { return none_const_t_return< std::vector<IVec2> >(); }
	const std::vector<IVec3>&  EffectParameter::get_vector_ivec3()  const { return none_const_t_return< std::vector<IVec3> >(); }
	const std::vector<IVec4>&  EffectParameter::get_vector_ivec4()  const { return none_const_t_return< std::vector<IVec4> >(); }

	const std::vector<Vec2>&   EffectParameter::get_vector_vec2()   const { return none_const_t_return< std::vector<Vec2> >(); }
	const std::vector<Vec3>&   EffectParameter::get_vector_vec3()   const { return none_const_t_return< std::vector<Vec3> >(); }
	const std::vector<Vec4>&   EffectParameter::get_vector_vec4()   const { return none_const_t_return< std::vector<Vec4> >(); }
	const std::vector<Mat3>&   EffectParameter::get_vector_mat3()   const { return none_const_t_return< std::vector<Mat3> >(); }
	const std::vector<Mat4>&   EffectParameter::get_vector_mat4()   const { return none_const_t_return< std::vector<Mat4> >(); }

	const std::vector<DVec2>&  EffectParameter::get_vector_dvec2()  const { return none_const_t_return< std::vector<DVec2> >(); }
	const std::vector<DVec3>&  EffectParameter::get_vector_dvec3()  const { return none_const_t_return< std::vector<DVec3> >(); }
	const std::vector<DVec4>&  EffectParameter::get_vector_dvec4()  const { return none_const_t_return< std::vector<DVec4> >(); }
	const std::vector<DMat3>&  EffectParameter::get_vector_dmat3()  const { return none_const_t_return< std::vector<DMat3> >(); }
	const std::vector<DMat4>&  EffectParameter::get_vector_dmat4()  const { return none_const_t_return< std::vector<DMat4> >(); }

	//struct by type
	struct EffectParameterTexture : public EffectParameter
	{
		//value
		Shared<Resource::Texture> m_value;
		//init
		EffectParameterTexture(Allocator* allocator) : EffectParameter(allocator)
		{
			m_value = Shared<Resource::Texture>(nullptr);
			m_type = EffectParameterType::PT_TEXTURE;
		}
		EffectParameterTexture(Allocator* allocator, Shared<Resource::Texture> value) : EffectParameter(allocator)
		{
			m_value = value;
			m_type = EffectParameterType::PT_TEXTURE;
		}
		//set value
		virtual void set(Shared<Resource::Texture> value) override
		{
			m_value = value;
		}
		//get value
		virtual Shared<Resource::Texture> get_texture() const override
		{
			return m_value;
		}
		//copy
		virtual Unique<EffectParameter> copy() const override
		{
			return std::move(MakeUnique<EffectParameterTexture>(m_allocator, m_allocator, m_value));
		}
		virtual void copy_to(EffectParameter* dest) const override
		{
			*((EffectParameterTexture*)dest) = (*this);
		}
		//ptr
		virtual void* value_ptr() override { return (void*)&m_value; }
	};
	struct EffectParameterVectorTexture : public EffectParameter
	{
		//value
		std::vector< Shared<Resource::Texture> > m_value;
		//init
		EffectParameterVectorTexture(Allocator* allocator) : EffectParameter(allocator)
		{
			m_value = {};
			m_type = EffectParameterType::PT_STD_VECTOR_TEXTURE;
		}
		EffectParameterVectorTexture(Allocator* allocator, const std::vector< Shared<Resource::Texture> >& value) : EffectParameter(allocator)
		{
			m_value = value;
			m_type = EffectParameterType::PT_STD_VECTOR_TEXTURE;
		}
		//set value
		virtual void set(const std::vector< Shared<Resource::Texture> >& value) override
		{
			m_value = value;
		}
		//get value
		virtual const std::vector< Shared<Resource::Texture> >& get_vector_texture() const override
		{
			return m_value;
		}
		//copy
		virtual Unique<EffectParameter> copy() const override
		{
			return std::move(MakeUnique<EffectParameterVectorTexture>(m_allocator, m_allocator, m_value));
		}
		virtual void copy_to(EffectParameter* dest) const override
		{
			*((EffectParameterVectorTexture*)dest) = (*this);
		}
		//ptr
		virtual void* value_ptr() override { return (void*)&m_value; }
	};
	//////////////////////////////////////////////////////////////////////////
    //utility
    #define DefineValueParameter(Classname,Type,defval)\
    struct Classname : public EffectParameter\
    {\
        Type m_value;\
        Classname(Allocator* allocator, Type value = defval) : EffectParameter(allocator) \
        {m_value = value; m_type = parameter_type_traits<Type>();}\
        virtual void set(Type value) override { m_value = value; }\
        virtual Type get_ ## Type () const override { return m_value; }\
        virtual Unique<EffectParameter> copy() const  override {  return std::move(MakeUnique< Classname >(m_allocator, m_allocator, m_value)); }\
	    virtual void copy_to(EffectParameter* dest) const override { *((Classname*)dest) = (*this); }\
        virtual void* value_ptr() override { return (void*)&m_value; }\
    }
    #define DefineObjectParameter(Classname,Type, getname, defval)\
    struct Classname : public EffectParameter\
    {\
        Type m_value;\
        Classname(Allocator* allocator, Type value = defval) : EffectParameter(allocator) \
        {m_value = value; m_type = parameter_type_traits<Type>();}\
        virtual void set(const Type& value) override { m_value = value; }\
        virtual const Type& get_ ## getname () const override { return m_value; }\
        virtual Unique<EffectParameter> copy() const  override {  return std::move(MakeUnique< Classname >(m_allocator, m_allocator, m_value)); }\
	    virtual void copy_to(EffectParameter* dest) const override { *((Classname*)dest) = (*this); }\
        virtual void* value_ptr() override { return (void*)&m_value; }\
    }
    #define DefineVectorObjectParameter(Classname,Type, getname)\
    struct Classname : public EffectParameter\
    {\
        std::vector< Type > m_value;\
        Classname(Allocator* allocator, const std::vector< Type >& value = {}) : EffectParameter(allocator) \
        {m_value = value; m_type = parameter_type_traits<Type>();}\
        virtual void set(const std::vector< Type >& value) override { m_value = value; }\
        virtual const std::vector< Type >& get_vector_ ## getname () const override { return m_value; }\
        virtual Unique<EffectParameter> copy() const  override {  return std::move(MakeUnique< Classname >(m_allocator, m_allocator, m_value)); }\
	    virtual void copy_to(EffectParameter* dest) const override { *((Classname*)dest) = (*this); }\
        virtual void* value_ptr() override { return (void*)&m_value; }\
    }
    //////////////////////////////////////////////////////////////////////////
    DefineValueParameter(EffectParameterInt, int, 0);
    DefineValueParameter(EffectParameterFloat, float, 0.0f);
    DefineValueParameter(EffectParameterDouble, double, 0.0);
    
    DefineObjectParameter(EffectParameterIVec2, IVec2, ivec2, IVec2(0));
    DefineObjectParameter(EffectParameterIVec3, IVec3, ivec3, IVec3(0));
    DefineObjectParameter(EffectParameterIVec4, IVec4, ivec4, IVec4(0));
    
    DefineObjectParameter(EffectParameterVec2, Vec2, vec2, Vec2(0));
    DefineObjectParameter(EffectParameterVec3, Vec3, vec3, Vec3(0));
    DefineObjectParameter(EffectParameterVec4, Vec4, vec4, Vec4(0));
    DefineObjectParameter(EffectParameterMat3, Mat3, mat3, Mat3(1));
    DefineObjectParameter(EffectParameterMat4, Mat4, mat4, Mat4(1));
    
    DefineObjectParameter(EffectParameterDVec2, DVec2, dvec2, DVec2(0));
    DefineObjectParameter(EffectParameterDVec3, DVec3, dvec3, DVec3(0));
    DefineObjectParameter(EffectParameterDVec4, DVec4, dvec4, DVec4(0));
    DefineObjectParameter(EffectParameterDMat3, DMat3, dmat3, DMat3(1));
    DefineObjectParameter(EffectParameterDMat4, DMat4, dmat4, DMat4(1));
    
    DefineVectorObjectParameter(EffectParameterVectorInt, int, int);
    DefineVectorObjectParameter(EffectParameterVectorFloat, float, float);
    DefineVectorObjectParameter(EffectParameterVectorDouble, double, double);
    
    DefineVectorObjectParameter(EffectParameterVectorIVec2, IVec2, ivec2);
    DefineVectorObjectParameter(EffectParameterVectorIVec3, IVec3, ivec3);
    DefineVectorObjectParameter(EffectParameterVectorIVec4, IVec4, ivec4);
    
    DefineVectorObjectParameter(EffectParameterVectorVec2, Vec2, vec2);
    DefineVectorObjectParameter(EffectParameterVectorVec3, Vec3, vec3);
    DefineVectorObjectParameter(EffectParameterVectorVec4, Vec4, vec4);
    DefineVectorObjectParameter(EffectParameterVectorMat3, Mat3, mat3);
    DefineVectorObjectParameter(EffectParameterVectorMat4, Mat4, mat4);
    
    DefineVectorObjectParameter(EffectParameterVectorDVec2, DVec2, dvec2);
    DefineVectorObjectParameter(EffectParameterVectorDVec3, DVec3, dvec3);
    DefineVectorObjectParameter(EffectParameterVectorDVec4, DVec4, dvec4);
    DefineVectorObjectParameter(EffectParameterVectorDMat3, DMat3, dmat3);
    DefineVectorObjectParameter(EffectParameterVectorDMat4, DMat4, dmat4);
    //////////////////////////////////////////////////////////////////////////    
	Unique<EffectParameter> EffectParameter::create(Allocator* allocator, EffectParameterType type)
	{
		switch (type)
		{
		case EffectParameterType::PT_TEXTURE:return std::move(MakeUnique< EffectParameterTexture >(allocator, allocator));
		case EffectParameterType::PT_INT:    return std::move(MakeUnique< EffectParameterInt >(allocator, allocator));
		case EffectParameterType::PT_FLOAT:  return std::move(MakeUnique< EffectParameterFloat >(allocator, allocator));
		case EffectParameterType::PT_DOUBLE: return std::move(MakeUnique< EffectParameterDouble >(allocator, allocator));

		case EffectParameterType::PT_IVEC2: return std::move(MakeUnique<EffectParameterIVec2>(allocator, allocator));
		case EffectParameterType::PT_IVEC3: return std::move(MakeUnique<EffectParameterIVec3>(allocator, allocator));
		case EffectParameterType::PT_IVEC4: return std::move(MakeUnique<EffectParameterIVec4>(allocator, allocator));

		case EffectParameterType::PT_VEC2: return std::move(MakeUnique<EffectParameterVec2>(allocator, allocator));
		case EffectParameterType::PT_VEC3: return std::move(MakeUnique<EffectParameterVec3>(allocator, allocator));
		case EffectParameterType::PT_VEC4: return std::move(MakeUnique<EffectParameterVec4>(allocator, allocator));
		case EffectParameterType::PT_MAT3: return std::move(MakeUnique<EffectParameterMat3>(allocator, allocator));
		case EffectParameterType::PT_MAT4: return std::move(MakeUnique<EffectParameterMat4>(allocator, allocator));

		case EffectParameterType::PT_DVEC2: return std::move(MakeUnique<EffectParameterDVec2>(allocator, allocator));
		case EffectParameterType::PT_DVEC3: return std::move(MakeUnique<EffectParameterDVec3>(allocator, allocator));
		case EffectParameterType::PT_DVEC4: return std::move(MakeUnique<EffectParameterDVec4>(allocator, allocator));
		case EffectParameterType::PT_DMAT3: return std::move(MakeUnique<EffectParameterDMat3>(allocator, allocator));
		case EffectParameterType::PT_DMAT4: return std::move(MakeUnique<EffectParameterDMat4>(allocator, allocator));

		case EffectParameterType::PT_STD_VECTOR_TEXTURE:return std::move(MakeUnique<EffectParameterVectorTexture>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_INT:    return std::move(MakeUnique<EffectParameterVectorInt>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_FLOAT:  return std::move(MakeUnique<EffectParameterVectorFloat>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_DOUBLE: return std::move(MakeUnique<EffectParameterVectorDouble>(allocator, allocator));

		case EffectParameterType::PT_STD_VECTOR_IVEC2: return std::move(MakeUnique<EffectParameterVectorIVec2>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_IVEC3: return std::move(MakeUnique<EffectParameterVectorIVec3>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_IVEC4: return std::move(MakeUnique<EffectParameterVectorIVec4>(allocator, allocator));

		case EffectParameterType::PT_STD_VECTOR_VEC2: return std::move(MakeUnique<EffectParameterVectorVec2>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_VEC3: return std::move(MakeUnique<EffectParameterVectorVec3>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_VEC4: return std::move(MakeUnique<EffectParameterVectorVec4>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_MAT3: return std::move(MakeUnique<EffectParameterVectorMat3>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_MAT4: return std::move(MakeUnique<EffectParameterVectorMat4>(allocator, allocator));

		case EffectParameterType::PT_STD_VECTOR_DVEC2: return std::move(MakeUnique<EffectParameterVectorDVec2>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_DVEC3: return std::move(MakeUnique<EffectParameterVectorDVec3>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_DVEC4: return std::move(MakeUnique<EffectParameterVectorDVec4>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_DMAT3: return std::move(MakeUnique<EffectParameterVectorDMat3>(allocator, allocator));
		case EffectParameterType::PT_STD_VECTOR_DMAT4: return std::move(MakeUnique<EffectParameterVectorDMat4>(allocator, allocator));
		default: return nullptr; break;
		}
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// EffectPass
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//Constructor
	EffectPass::EffectPass() {}
	EffectPass::~EffectPass() {}

	//enable pass effect
	void EffectPass::bind(  Render::Context&  render
						  , const EffectPassInputs& inputs
                          , EffectParameters* params) const
	{
		//bind
		bind(render, params);
		//buffers (camera/model/transofrm)
		if (inputs.m_camera && m_uniform_camera && m_uniform_camera->is_valid())
		{
			m_uniform_camera->bind(inputs.m_camera);
		}
		if (inputs.m_transform && m_uniform_transform && m_uniform_transform->is_valid())
		{
			m_uniform_transform->bind(inputs.m_transform);
		}
		//lights
		if (m_uniform_ambient_light && m_uniform_ambient_light->is_valid())
		{
			m_uniform_ambient_light->set(inputs.m_ambient_light);
		}
		if (inputs.m_direction_light && m_uniform_direction && m_uniform_direction->is_valid())
		{
			m_uniform_direction->bind(inputs.m_direction_light);
		}
		if (inputs.m_point_light && m_uniform_point && m_uniform_point->is_valid())
		{
			m_uniform_point->bind(inputs.m_point_light);
		}
		if (inputs.m_spot_light && m_uniform_spot && m_uniform_spot->is_valid())
		{
			m_uniform_spot->bind(inputs.m_spot_light);
		}
		//shadow
		if (inputs.m_shadow_map && m_uniform_shadow_map /*&& m_uniform_shadow_map->is_valid*/)
		{
			m_uniform_shadow_map->set(inputs.m_shadow_map);
		}
		if (inputs.m_direction_shadow_light && m_uniform_direction_shadow && m_uniform_direction_shadow->is_valid())
		{
			m_uniform_direction_shadow->bind(inputs.m_direction_shadow_light);
		}
		if (inputs.m_point_shadow_light && m_uniform_point_shadow && m_uniform_point_shadow->is_valid())
		{
			m_uniform_point_shadow->bind(inputs.m_point_shadow_light);
		}
		if (inputs.m_spot_shadow_light && m_uniform_spot_shadow && m_uniform_spot_shadow->is_valid())
		{
			m_uniform_spot_shadow->bind(inputs.m_spot_shadow_light);
		}
	}

	void EffectPass::bind(Render::Context& render, EffectParameters* params) const
	{
		//bind
		render.set_blend_state(m_blend);
		render.set_cullface_state(m_cullface);
		render.set_depth_buffer_state(m_depth);
		//bind shader
		m_shader->bind();
		//test
		if (!params) return;
		//uniform
	    for (size_t i = 0; i != m_uniform.size(); ++i)
		{
			auto& param = (*params)[m_param_id[i]];
			//uniform value
			switch (param->get_type())
			{
			default:
			case EffectParameterType::PT_NONE: /* void */ break;
			//uniform
			case EffectParameterType::PT_TEXTURE:  
				if (param->get_texture())
				{
					m_uniform[i]->set(param->get_texture()->get_context_texture());
				}
				else
				{
					// Failed : TODO use app logger
					printf("Render: EffectPass::bind(texture): error to set a texture");
				}
			break;
			case EffectParameterType::PT_INT:       m_uniform[i]->set(param->get_int()); break;
			case EffectParameterType::PT_FLOAT:     m_uniform[i]->set(param->get_float()); break;
			case EffectParameterType::PT_DOUBLE:    m_uniform[i]->set(param->get_double()); break;

			case EffectParameterType::PT_IVEC2:     m_uniform[i]->set(param->get_ivec2()); break;
			case EffectParameterType::PT_IVEC3:     m_uniform[i]->set(param->get_ivec3()); break;
			case EffectParameterType::PT_IVEC4:     m_uniform[i]->set(param->get_ivec4()); break;

			case EffectParameterType::PT_VEC2:      m_uniform[i]->set(param->get_vec2()); break;
			case EffectParameterType::PT_VEC3:      m_uniform[i]->set(param->get_vec3()); break;
			case EffectParameterType::PT_VEC4:      m_uniform[i]->set(param->get_vec4()); break;
			case EffectParameterType::PT_MAT3:      m_uniform[i]->set(param->get_mat3()); break;
			case EffectParameterType::PT_MAT4:      m_uniform[i]->set(param->get_mat4()); break;

			case EffectParameterType::PT_DVEC2:      m_uniform[i]->set(param->get_dvec2()); break;
			case EffectParameterType::PT_DVEC3:      m_uniform[i]->set(param->get_dvec3()); break;
			case EffectParameterType::PT_DVEC4:      m_uniform[i]->set(param->get_dvec4()); break;
			case EffectParameterType::PT_DMAT3:      m_uniform[i]->set(param->get_dmat3()); break;
			case EffectParameterType::PT_DMAT4:      m_uniform[i]->set(param->get_dmat4()); break;

#if 0
			case EffectParameterType::PT_STD_VECTOR_TEXTURE:    m_uniform[i]->set(param->get_vector_texture()/* todo, cast */); break;
#endif

			case EffectParameterType::PT_STD_VECTOR_INT:    m_uniform[i]->set(param->get_vector_int()); break;
			case EffectParameterType::PT_STD_VECTOR_FLOAT:  m_uniform[i]->set(param->get_vector_float()); break;
			case EffectParameterType::PT_STD_VECTOR_DOUBLE: m_uniform[i]->set(param->get_vector_double()); break;

			case EffectParameterType::PT_STD_VECTOR_IVEC2: m_uniform[i]->set(param->get_vector_ivec2()); break;
			case EffectParameterType::PT_STD_VECTOR_IVEC3: m_uniform[i]->set(param->get_vector_ivec3()); break;
			case EffectParameterType::PT_STD_VECTOR_IVEC4: m_uniform[i]->set(param->get_vector_ivec4()); break;

			case EffectParameterType::PT_STD_VECTOR_VEC2: m_uniform[i]->set(param->get_vector_vec2()); break;
			case EffectParameterType::PT_STD_VECTOR_VEC3: m_uniform[i]->set(param->get_vector_vec3()); break;
			case EffectParameterType::PT_STD_VECTOR_VEC4: m_uniform[i]->set(param->get_vector_vec4()); break;
			case EffectParameterType::PT_STD_VECTOR_MAT3: m_uniform[i]->set(param->get_vector_mat3()); break;
			case EffectParameterType::PT_STD_VECTOR_MAT4: m_uniform[i]->set(param->get_vector_mat4()); break;

			case EffectParameterType::PT_STD_VECTOR_DVEC2: m_uniform[i]->set(param->get_vector_dvec2()); break;
			case EffectParameterType::PT_STD_VECTOR_DVEC3: m_uniform[i]->set(param->get_vector_dvec3()); break;
			case EffectParameterType::PT_STD_VECTOR_DVEC4: m_uniform[i]->set(param->get_vector_dvec4()); break;
			case EffectParameterType::PT_STD_VECTOR_DMAT3: m_uniform[i]->set(param->get_vector_dmat3()); break;
			case EffectParameterType::PT_STD_VECTOR_DMAT4: m_uniform[i]->set(param->get_vector_dmat4()); break;
			}
		}
	
	}

	//disable pass effect
	void EffectPass::unbind() const
	{
		m_shader->unbind();
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// EffectTechnique
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//info
	size_t EffectTechnique::size()
	{
		return m_passes.size();
	}

	//alloc
	void EffectTechnique::resize(size_t n)
	{
		m_passes.resize(n);
	}

	void EffectTechnique::reserve(size_t n)
	{
		m_passes.reserve(n);
	}

	//push
	void EffectTechnique::push_back(const EffectPass&& pass)
	{
		m_passes.push_back(std::move(pass));
	}

	//operator
	EffectPass& EffectTechnique::operator[](size_t i)
	{
		return m_passes[i];
	}

	const EffectPass& EffectTechnique::operator[](size_t i) const
	{
		return m_passes[i];
	}

	//iterators
	EffectTechnique::PassListIt EffectTechnique::begin()
	{
		return m_passes.begin();
	}

	EffectTechnique::PassListIt EffectTechnique::end()
	{
		return m_passes.end();
	}

	EffectTechnique::PassListCIt EffectTechnique::begin() const
	{
		return m_passes.begin();
	}

	EffectTechnique::PassListCIt EffectTechnique::end() const
	{
		return m_passes.end();
	}

	EffectPass& EffectTechnique::back()
	{
		return m_passes.back();
	}
	const EffectPass& EffectTechnique::back() const
	{
		return m_passes.back();
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// EffectTechnique
	/////////////////////////////////////////////////////////////////////////////////////////////////////
	//contructor
	Effect::Effect(Allocator* allocator)
	: m_allocator(allocator)
	{}
	Effect::~Effect(){}

	//get technique
	EffectTechnique* Effect::technique(const std::string& technique_name)
	{
		auto it_tech = m_techniques_map.find(technique_name);
		if (it_tech != m_techniques_map.end()) return &it_tech->second;
		return nullptr;
	}
	const EffectTechnique* Effect::technique(const std::string& technique_name) const
	{
		auto it_tech = m_techniques_map.find(technique_name);
		if (it_tech != m_techniques_map.end()) return &it_tech->second;
		return nullptr;
	}

	//get parameters
	EffectParameters* Effect::parameters()
	{
		return &m_parameters;
	}

	//get parameter
	EffectParameter* Effect::parameter(int parameter_id) const
	{
		if (m_parameters.size() > (size_t)parameter_id)
			return m_parameters[parameter_id].get();
		return nullptr;
	}

	EffectParameter* Effect::parameter(const std::string& parameter_name) const
	{
		auto it_param = m_parameters_map.find(parameter_name);
		if (it_param != m_parameters_map.end()) return parameter(it_param->second);
		return nullptr;
	}
	
	Unique<EffectParameters> Effect::copy_all_parameters() const
	{
		//alloc vector
		auto new_params = MakeUnique<EffectParameters>(m_allocator, m_parameters.size());
		//copy all
		for (size_t i = 0; i != m_parameters.size(); ++i)
		{
			(*new_params)[i] = std::move(m_parameters[i]->copy());
		}
		return std::move(new_params);
	}
	
	const std::string&  Effect::parameter_name(int parameter_id) const
	{
		for (auto element : m_parameters_map) if (element.second == parameter_id) return element.first;
		return std::string();
	}
	//get id
	int Effect::parameter_id(const std::string& parameter_name)
	{
		auto it_param = m_parameters_map.find(parameter_name);
		if (it_param != m_parameters_map.end()) return it_param->second;
		return -1;
	}

	//copy pass (import)
	bool Effect::import_technique(const Effect& effect,const std::string& name)
	{
		//get
		const EffectTechnique* in_technique = effect.technique(name);
		//test
		if (!in_technique) return false;
		//copy
		auto& new_technique = (m_techniques_map[name] = *in_technique);
		//rebuild ids
		for_each_pass_build_params_id(new_technique);
		return true;
	}

	//copy pass (import) 
	bool Effect::import_techniques(const Effect& effect)
	{
		for (auto technique : effect.techniques())
		{
			if (!import_technique(effect, technique.first))
				return false;
		}
		return true;
	}

	//add
	int Effect::add_parameter(int id, const std::string& name, Unique<EffectParameter>&& parameter)
	{
		parameter->m_id = id; 
		m_parameters[id] = std::move(parameter);
		m_parameters_map[name] = id;
		return id;
	}

	//help
	void Effect::for_each_pass_build_params_id()
	{
		for (auto& technique : m_techniques_map)
			for_each_pass_build_params_id(technique.second);
	}
	void Effect::for_each_pass_build_params_id(EffectTechnique& technique)
	{
		for (EffectPass& pass : technique)
		{
			//clear
			pass.m_param_id.clear();
			pass.m_uniform.clear();
			//build
			for (auto parameter : m_parameters_map)
			{
				//get
				Render::Uniform* u_shader = pass.m_shader->uniform(parameter.first);
				//test
				if (u_shader)
				{
					//push
					pass.m_param_id.push_back(parameter.second);
					pass.m_uniform.push_back(u_shader);
				}
			}
		}
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////
}
}
