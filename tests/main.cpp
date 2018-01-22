//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include <Square/Square.h>
#include <Square/Driver/Window.h>
#include <Square/Driver/Input.h>
#include <iostream>
#include <sstream>
#include <fstream>

class Sprite : public Square::Scene::Component
{
public:
	SQUARE_OBJECT(Sprite)

	using Vec2     = Square::Vec2;
	using SRObject = Square::Shared < Square::ResourceObject >;
	using STexture = Square::Shared < Square::Resource::Texture >;
	
	Vec2    m_scale; //scale
	Vec2    m_pos;   //2D pos
	STexture m_texture;

    Sprite(){ }
    
    virtual void on_attach(Square::Scene::Actor& entity) override
    {
        
    }
    virtual void on_deattch() override
    {
        
    }
    virtual void on_message(const Square::Scene::Message& msg) override
    {
        
    }
    
    static void object_registration(Square::Context& context)
    {
		context.add_object<Sprite>();
        context.add_attributes<Sprite>(Square::attribute_field< Sprite >("scale", Square::Vec2(0,0), &Sprite::m_scale));
		context.add_attributes<Sprite>(Square::attribute_field< Sprite >("pos", Square::Vec2(0, 0), &Sprite::m_pos));
		context.add_attributes<Sprite>(Square::attribute_field< Sprite >("texture", STexture(nullptr), &Sprite::m_texture));
		//std::cout << Square::variant_traits<STexture>() << std::endl;
    }
};
SQUARE_CLASS_OBJECT_REGISTRATION(Sprite);


class Body : public Square::Scene::Component
{
public:
    SQUARE_OBJECT(Body)

	Square::Vec3 m_gravity;
	float	     m_mass;
	int 	     m_nshape;

	void  set_gravity(const Square::Vec3& g){ m_gravity = g; }
	const Square::Vec3& get_gravity() const { return m_gravity; }

	void  set_mass(float m)       { m_mass = m; }
	float get_mass()		const { return m_mass; }

	static void object_registration(Square::Context& context)
	{
		context.add_object<Body>();
        context.add_attributes<Body>(Square::attribute_method< Body >("gravity", Square::Vec3(0,0,0), &Body::get_gravity, &Body::set_gravity));
        context.add_attributes<Body>(Square::attribute_field< Body >("mass", float(0), &Body::m_mass));
        context.add_attributes<Body>(Square::attribute_field< Body >("nshape", int(0), &Body::m_nshape));
	}
};
SQUARE_CLASS_OBJECT_REGISTRATION(Body);

#include <Xsc/Xsc.h>
#include <fstream> 
#include <sstream> 

class CompilerLog : public Xsc::Log
{
public:
	//! Submits the specified report.
	virtual void SubmitReport(const Xsc::Report& report)
	{
		switch (report.Type())
		{
			case Xsc::ReportTypes::Info:
				m_info_list.push_back(report);
				break;
			case Xsc::ReportTypes::Warning:
				m_warning_list.push_back(report);
				break;
			case Xsc::ReportTypes::Error:
				m_error_list.push_back(report);
			break;
		}
	}

	//errors to string
	std::string get_errors() const
	{
		std::stringstream ss;
		for (auto error : m_error_list)
		{
			ss << error.Message();
			ss << std::endl;
		}
		return ss.str();
	}

	//warning to string
	std::string get_warning() const
	{
		std::stringstream ss;
		for (auto warning : m_warning_list)
		{
			ss << warning.Message();
			ss << std::endl;
		}
		return ss.str();
	}

	//warning to string
	std::string get_info() const
	{
		std::stringstream ss;
		for (auto info : m_info_list)
		{
			ss << info.Message();
			ss << std::endl;
		}
		return ss.str();
	}

protected:
	//error info
	std::list<Xsc::Report> m_error_list;
	std::list<Xsc::Report> m_warning_list;
	std::list<Xsc::Report> m_info_list;

};

class Game01 : public Square::AppInterface
{
public:

	void key_event(Square::Video::KeyboardEvent key, short mode, Square::Video::ActionEvent action)
	{
		switch (key)
		{
		case Square::Video::KEY_Q: m_loop = false; break;
		case Square::Video::KEY_W: Square::Application::instance()->fullscreen(false); break;
		case Square::Video::KEY_F: Square::Application::instance()->fullscreen(true); break;
		default: break;
		}
	}

    void start()
    {
		using namespace Square;
		using namespace Square::Data;
		using namespace Square::Scene;
		using namespace Square::Resource;
		//rs file
        context().add_resource_file("example.png");
		
		//test
		Scene::Actor player;
		player.translation({ 10,0,0 });
		player.component<Sprite>()->m_scale = { 2,2 };
		player.component<Sprite>()->m_pos = { 100,0 };
		player.component<Sprite>()->m_texture = context().resource<Texture>("example");
		player.component<Body>()->m_gravity = { 0,10, 0 };
		{
			std::ofstream ofile("agent.bin", std::ios::binary | std::ios::out);
			ArchiveBinWrite out(ofile);
			player.serialize(out);
		}
		{
			std::ifstream ifile("agent.bin", std::ios::binary | std::ios::in);
			ArchiveBinRead in(ifile);
			player.deserialize(in);
		}
		std::cout << "pos "
				  << player.component<Sprite>()->m_pos.x
				  << ", "
				  << player.component<Sprite>()->m_pos.y;
		std::cout << std::endl;
		std::cout << "scale "
				  << player.component<Sprite>()->m_scale.x
				  << ", "
				  << player.component<Sprite>()->m_scale.y;
		std::cout << std::endl;
		std::cout << "texture "
				  << player.component<Sprite>()->m_texture->get_width()
				  << ", "
				  << player.component<Sprite>()->m_texture->get_height();
		std::cout << std::endl;
		//in/out
		std::stringstream osourceVS;
		std::stringstream osourceFS;
		std::string ssrouce(R"HLSL(  
			#define IVec2 int2
			#define IVec3 int3
			#define IVec4 int4
			#define IMat3 int3x3
			#define IMat4 int4x4

			#define Vec2 float2
			#define Vec3 float3
			#define Vec4 float4
			#define Mat3 float3x3
			#define Mat4 float4x4

			#define DVec2 double2
			#define DVec3 double3
			#define DVec4 double4
			#define DMat3 double3x3
			#define DMat4 double4x4

			struct VS_INPUT
			{
				Vec3 vPos   : POSITION;
				Vec3 vColor : COLOR0;
			};

			struct VS_OUT
			{
				Vec4 Position : SV_POSITION;  // interpolated vertex position (system value)
				Vec4 Color    : COLOR0;       // interpolated diffuse color
			};

			cbuffer ModelViewProjectionConstantBuffer : register(b0)
			{
				Mat4 mWorld;      // world matrix for object
				Mat4 View;        // view matrix
				Mat4 Projection;  // projection matrix
			};

			VS_OUT vertex(VS_INPUT input) 
			{    
				VS_OUT Output;

				Vec4 pos = float4(input.vPos, 1.0f);

				// Transform the position from object space to homogeneous projection space
				pos = mul(pos, mWorld);
				pos = mul(pos, View);
				pos = mul(pos, Projection);
				Output.Position = pos;

				// Just pass through the color data
				Output.Color = float4(input.vColor, 1.0f);

				return Output;
			}

			Vec4 fragment(VS_OUT input) : SV_Target
			{    
				return input.Color;
			}
		)HLSL");
		//compile shader
		Xsc::ShaderInput inputVS;
		{
			inputVS.sourceCode = std::make_shared<std::stringstream>(ssrouce);
			inputVS.entryPoint = "vertex";
			inputVS.shaderVersion = Xsc::InputShaderVersion::HLSL5;
			inputVS.shaderTarget = Xsc::ShaderTarget::VertexShader;
		}
		Xsc::ShaderInput inputFS;
		{
			inputFS.sourceCode = std::make_shared<std::stringstream>(ssrouce);
			inputFS.entryPoint = "fragment";
			inputFS.shaderVersion = Xsc::InputShaderVersion::HLSL5;
			inputFS.shaderTarget = Xsc::ShaderTarget::FragmentShader;
		}
		//shader output
		Xsc::ShaderOutput outputVS;
		{
			outputVS.sourceCode = &osourceVS;
			outputVS.shaderVersion = Xsc::OutputShaderVersion::GLSL410;
		}
		Xsc::ShaderOutput outputFS;
		{
			outputFS.sourceCode = &osourceFS;
			outputFS.shaderVersion = Xsc::OutputShaderVersion::GLSL410;
		}
		{
			CompilerLog logs;
			bool result = Xsc::CompileShader(inputVS, outputVS, &logs);
			if (result)
			{
				std::cerr << "info: " << std::endl << logs.get_info();
				std::cerr << "warning: " << std::endl << logs.get_warning();
				std::cout << "success:" << std::endl << osourceVS.str() << std::endl;
			}
			else
			{
				std::cerr << "warning: " << std::endl << logs.get_warning();
				std::cerr << "fail: " << std::endl << logs.get_errors();
			}
		}
		{
			CompilerLog logs;
			bool result = Xsc::CompileShader(inputFS, outputFS, &logs);
			if (result)
			{
				std::cerr << "info: " << std::endl << logs.get_info();
				std::cerr << "warning: " << std::endl << logs.get_warning();
				std::cout << "success:" << std::endl << osourceFS.str() << std::endl;
			}
			else
			{
				std::cerr << "warning: " << std::endl << logs.get_warning();
				std::cerr << "fail: " << std::endl << logs.get_errors();
			}
		}
			
    }
    bool run(double dt)
    {
        return m_loop;
    }
    bool end()
    {
        return true;
    }
private:
    
    bool m_loop = true;
    
};

int main()
{
    using namespace Square;
    using namespace Square::Data;
    using namespace Square::Scene;
	//Create square application
	Application app;
	//test
    app.execute(
      WindowSizePixel({ 1280, 768 })
    , WindowMode::NOT_RESIZABLE
    , 4, 1
    , "test"
    , new Game01()
    );
    //End
    return 0;
}
