//
//  Square
//
//  Created by Gabriele on 18/10/17.
//  Copyright © 2016 Gabriele. All rights reserved.
//
#pragma once
#include <cmath>

//DEPTH [0,1] Vulakn / DirectX like//
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_RIGHT_HANDED 

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/quaternion.hpp>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <glm/gtc/matrix_access.hpp> 

#include <glm/gtx/matrix_operation.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/ext.hpp>

namespace Square
{
    using  Vec2 = glm::vec2;
    using DVec2 = glm::dvec2;
    using IVec2 = glm::ivec2;
    
    using  Vec3 = glm::vec3;
    using DVec3 = glm::dvec3;
    using IVec3 = glm::ivec3;
    
    using  Vec4 = glm::vec4;
    using DVec4 = glm::dvec4;
    using IVec4 = glm::ivec4;
    
    using  Mat3 = glm::mat3;
    using DMat3 = glm::dmat3;
    
    using  Mat4 = glm::mat4;
    using DMat4 = glm::dmat4;
    
    using Quat  = glm::quat;
    using DQuat = glm::dquat;
    
    template < typename Scalar, glm::precision P = glm::defaultp >  using  TVec2 = glm::tvec2< Scalar, P >;
    template < typename Scalar, glm::precision P = glm::defaultp >  using  TVec3 = glm::tvec3< Scalar, P >;
    template < typename Scalar, glm::precision P = glm::defaultp >  using  TVec4 = glm::tvec4< Scalar, P >;
    template < typename Scalar, glm::precision P = glm::defaultp >  using  TMat3 = glm::tmat3x3< Scalar, P >;
    template < typename Scalar, glm::precision P = glm::defaultp >  using  TMat4 = glm::tmat4x4< Scalar, P >;

    namespace Constants
    {
        template <class T>
        inline T pi()
        {
            return glm::pi<T>();
        }
		template <class T>
		inline T pi2()
		{
			return glm::pi<T>()*((T)2.0);
		}
		template <class T>
		inline T e()
		{
			return glm::e<T>();
		}
		template <class T>
		inline T golden_ratio()
		{
			return glm::golden_ratio<T>();
		}
        template <class T>
        inline const T& identity()
        {
            static T I(1.0);
            return I;
        }
    }
    

	template < class T >
	T max(const T& a, const T& b)
	{
		return b < a ? a : b;
	}

	template < class T, typename ...ARGS>
	T max(const T& a, const T& b, ARGS... args)
	{ 
		return max(max<T>(a,b), args...);
	}

	template < class T >
	T min(const T& a, const T& b)
	{
		return a < b ? a : b;
	}

	template < class T, typename ...ARGS>
	T min(const T& a, const T& b, ARGS... args) 
	{ 
		return min(min<T>(a, b), args...);
	}

    template < class T >
    inline T dot(const T& a, const T& b)
    {
        return glm::dot(a, b);
    }
    
    template < class T >
    inline T cross(const T& a, const T& b)
    {
        return glm::cross(a, b);
    }
    
    template < class T >
    inline T normalize(const T& a)
    {
        return glm::normalize(a);
    }
    
    template < class T >
    inline T length(const T& a)
    {
        return glm::length(a);
    }
    
    template < class T >
    inline T ceil(const T& a)
    {
        return glm::ceil(a);
    }
    
    template < class T >
    inline T floor(const T& a)
    {
        return glm::floor(a);
    }
    
    template < class T >
    inline T abs(const T& a)
    {
        return glm::abs(a);
    }
    
    template < class T >
    inline T clamp(const T& a,const T& min, const T&  max)
    {
        return glm::clamp(a,min,max);
    }
    
    template < class T, class F >
    inline T mix(const T& a,const T& b, const F&  f)
    {
        return glm::mix(a,b,f);
    }

	template < class T, class F >
	inline T lerp(const T& a, const T& b, const F&  f)
	{
		return glm::mix(a, b, f);
	}
    
    template < class T >
    inline T traspose(const T& a)
    {
        return glm::transpose(a);
    }
    
    template < class T >
    inline T inverse(const T& a)
    {
        return glm::inverse(a);
    }
    
    template < class T >
    inline T conjugate(const T& a)
    {
        return glm::conjugate(a);
    }
    
    template < class T >
    inline T diagonal3x3(const T& a)
    {
        return glm::diagonal3x3(a);
    }
    
    template < class T >
    inline T diagonal4x4(const T& a)
    {
        return glm::diagonal4x4(a);
    }
    
    template < class T >
    inline T radians(const T& a)
    {
        return glm::radians(a);
    }
    
    template < class T >
    inline T degrees(const T& a)
    {
        return glm::degrees(a);
    }

	template < class T >
	inline auto row(const T& a, size_t i) -> decltype( glm::row(a, i) )
	{
		return glm::row(a, i);
	}

	template < class T >
	inline auto column(const T& a, size_t i) -> decltype( glm::column(a, i) )
	{
		return glm::column(a, i);
	}
    
    template < class T >
    inline auto value_ptr(const T& q_m_v) -> decltype( glm::value_ptr(q_m_v) )
    {
        return glm::value_ptr(q_m_v);
    }
    
    template < class T >
    inline auto quat_cast(const T& q_m) -> decltype( glm::quat_cast(q_m) )
    {
        return glm::quat_cast(q_m);
    }

	template < class T >
	inline auto mat3_cast(const T& q_v) -> decltype(glm::mat3_cast(q_v))
	{
		return glm::mat3_cast(q_v);
	}

    template < class T >
    inline auto mat4_cast(const T& q_v) -> decltype(  glm::mat4_cast(q_v) )
    {
        return glm::mat4_cast(q_v);
    }
    
	template < class T >
	inline auto to_quat(const T& q_m) -> decltype(glm::toQuat(q_m))
	{
		return glm::toQuat(q_m);
	}

	template < class T >
	inline auto to_mat3(const T& q_v) -> decltype(glm::toMat3(q_v))
	{
		return glm::toMat3(q_v);
	}

	template < class T >
	inline auto to_mat4(const T& q_v) -> decltype(glm::toMat4(q_v))
	{
		return glm::toMat4(q_v);
	}
    
    /*
    Quaternion
    */
    template < typename Scalar, glm::precision P = glm::defaultp >
    inline auto euler_to_quat(const Scalar& x, const Scalar& y,const Scalar& z) -> glm::tquat< Scalar, P >
    {
        return glm::eulerAngleXYZ<Scalar>(x,y,z);
    }
    template < typename Scalar, glm::precision P = glm::defaultp >
    inline auto euler_to_quat(const glm::tvec3< Scalar, P >& vec) -> glm::tquat< Scalar, P >
    {
        return glm::eulerAngleXYZ<Scalar,P>(vec.x,vec.y,vec.z);
    }
    
    /*
    * Eigenvalues rotate
    */
    template < typename Scalar = float >
    inline void eigenvalues_rotate(TMat3< Scalar >& mat, const Scalar& c, const Scalar& s, size_t i0, size_t j0, size_t i1, size_t j1)
    {
		Scalar a = c * mat[i0][j0] - s * mat[i1][j1];
		Scalar b = s * mat[i0][j0] + c * mat[i1][j1];
        mat[i0][j0] = (Scalar)a;
        mat[i1][j1] = (Scalar)b;
    }
    
    /**
     * Diagonize matrix using Jacobi rotations.
     * @remark Method does not check if matrix is diagonizable.
     * Passing non diagonizable matrix and infinite max_iter (= -1)
     * May result in infinite loop.
     */
    template < typename Scalar = float >
    inline TVec3< Scalar > eigenvalues_jacobi(TMat3< Scalar >& mat, size_t max_iter, TMat3< Scalar >& E)
    {
        //return vector
        TVec3< Scalar > ret;
        
        // initial eigenvalues
        ret[0] = mat[0][0];
        ret[1] = mat[1][1];
        ret[2] = mat[2][2];
        
        mat[0][0] = 1.0;
        mat[1][1] = 1.0;
        mat[2][2] = 1.0;
        
        E = TMat3< Scalar >(1.0);
        
        for(size_t z = 0; z!=max_iter; ++z)
        {
            for (size_t i = 0; i != 3; ++i)
            {
                for (size_t j = i + 1; j < 3; j++)
                {
                    Scalar mii = ret[i];
                    Scalar mjj = ret[j];
                    Scalar mij = mat[i][j];
                    Scalar phi = Scalar(0.5) * std::atan2(Scalar(2.0) * mij, mjj - mii);
                    Scalar c = std::cos(phi);
                    Scalar s = std::sin(phi);
                    Scalar mii1 = c * c * mii - 2 * s * c * mij + s * s * mjj;
                    Scalar mjj1 = s * s * mii + 2 * s * c * mij + c * c * mjj;
                    if (std::abs(mii - mii1) < 0.00001 || (mjj - mjj1) < 0.00001) {
                        ret[i]    = (Scalar)mii1;
                        ret[j]    = (Scalar)mjj1;
                        mat[i][j] = (Scalar)0.0;
                        for (size_t k = 0;     k < i; ++k) eigenvalues_rotate<Scalar>(mat, c, s, k, i, k, j);
                        for (size_t k = i + 1; k < j; ++k) eigenvalues_rotate<Scalar>(mat, c, s, i, k, k, j);
                        for (size_t k = j + 1; k < 3; ++k) eigenvalues_rotate<Scalar>(mat, c, s, i, k, j, k);
                        //rotate eigenvectors
                        for (size_t k = 0; k < 3; k++) eigenvalues_rotate(E, c, s, k, i, k, j);
                    }
                }
            }
        }
        return ret;
    }
}
