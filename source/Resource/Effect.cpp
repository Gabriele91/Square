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
#include "Square/Data/ParserEffect.h"
#include "Square/Resource/Effect.h"
#include "Square/Resource/Texture.h"
#include "Square/Resource/Shader.h"
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
		//alias
		using EffectPass          = Render::EffectPass;
		using EffectTechnique     = Render::EffectTechnique;
		using EffectParameter     = Render::EffectParameter;
		using EffectParameterType = Render::EffectParameterType;
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
                case EffectParameterType::PT_TEXTURE:
					add_parameter((int)i, e_context.m_parameters[i].m_name, EffectParameter::create(context().resource<Texture>(e_context.m_parameters[i].m_resources[0])));
                break;
                case EffectParameterType::PT_STD_VECTOR_TEXTURE:
                {
                    //vector of texture
                    std::vector< Shared<Texture> > v_textures;
                    //get all texture
                    for(const std::string& tex_name : e_context.m_parameters[i].m_resources)
                        v_textures.push_back( context().resource<Texture>(tex_name) );
					//add
					add_parameter((int)i, e_context.m_parameters[i].m_name, EffectParameter::create(v_textures));
                }
                break;
                default:
					add_parameter((int)i, e_context.m_parameters[i].m_name, e_context.m_parameters[i].m_paramter->copy());
                break;
            }
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
        queue(ptr_sub_effect->m_queue);
        //n_pass
        size_t n_techniques_parser = ptr_sub_effect->m_techniques.size();
        //add tech
        for (size_t t = 0; t != n_techniques_parser; ++t)
        {
            //add into map
            EffectTechnique& this_technique = m_techniques_map[ptr_sub_effect->m_techniques[t].m_name];
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
                shader_define_rendering current_shader_def;
                //pass
				while (light_sub_pass)
                {
                    //1 pass for light
					if (light_sub_pass & Parser::Effect::LT_COLOR)
					{
						current_shader_def = DEF_RENDERING_COLOR;
						light_sub_pass ^= Parser::Effect::LT_COLOR;
					}
					if(light_sub_pass & Parser::Effect::LT_AMBIENT)
                    {
                        current_shader_def = DEF_RENDERING_AMBIENT_LIGHT;
                        light_sub_pass    ^= Parser::Effect::LT_AMBIENT;
                    }
                    else if(light_sub_pass & Parser::Effect::LT_DIRECTION)
                    {
                        current_shader_def = DEF_RENDERING_DIRECTION_LIGHT;
                        light_sub_pass    ^= Parser::Effect::LT_DIRECTION;
                    }
                    else if(light_sub_pass & Parser::Effect::LT_POINT)
                    {
                        current_shader_def = DEF_RENDERING_POINT_LIGHT;
                        light_sub_pass    ^= Parser::Effect::LT_POINT;
                    }
                    else if(light_sub_pass & EffectPass::LT_SPOT)
                    {
                        current_shader_def = DEF_RENDERING_SPOT_LIGHT;
                        light_sub_pass    ^= Parser::Effect::LT_SPOT;
                    }
					//not suppoted
					else
					{
						break;
					}
                    //add pass
                    this_technique.push_back(EffectPass());
                    //pass
                    EffectPass& this_pass = this_technique.back();
                    //get all values
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
                            std::make_tuple(std::string("version"), std::to_string(ptr_sub_effect->m_requirement.m_shader_version) ),
							std::make_tuple(std::string("define"), shader_define_table[current_shader_def] )
                        };
                        //shader
                        this_pass.m_shader = MakeShared<Shader>(context());
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
                                debug_preproc += "#"+std::get<0>(preproc)+" "+std::get<1>(preproc)+"\t";
                            }
                            //output
							context().add_wrong("Effect: " + path);
							context().add_wrong("Error from technique: " + ptr_sub_effect->m_techniques[t].m_name + ", pass[" + std::to_string(p) + "] ");
							context().add_wrong("Error technique preproces: " + debug_preproc);
                            return false;
                        }
                    }
                    //default uniform
                    this_pass.m_uniform_camera    = this_pass.m_shader->constant_buffer("Camera");
                    this_pass.m_uniform_transform = this_pass.m_shader->constant_buffer("Transform");
                    //try
                    if(!this_pass.m_uniform_camera) this_pass.m_uniform_camera = this_pass.m_shader->constant_buffer("camera");
                    if(!this_pass.m_uniform_transform) this_pass.m_uniform_transform = this_pass.m_shader->constant_buffer("transform");
                    //lights uniforms
                    switch (current_shader_def)
                    {
                        case DEF_RENDERING_AMBIENT_LIGHT:
                            this_pass.m_uniform_ambient_light = this_pass.m_shader->uniform("AmbientLight");
							if (!this_pass.m_uniform_ambient_light) this_pass.m_uniform_ambient_light = this_pass.m_shader->uniform("light");
							this_pass.m_support_light = EffectPass::LT_AMBIENT;
                            break;
						case DEF_RENDERING_DIRECTION_LIGHT:
							this_pass.m_uniform_direction = this_pass.m_shader->constant_buffer("DirectionLight");
							if (!this_pass.m_uniform_direction) this_pass.m_uniform_direction = this_pass.m_shader->constant_buffer("direction_light");
							if (!this_pass.m_uniform_direction) this_pass.m_uniform_direction = this_pass.m_shader->constant_buffer("light");
							this_pass.m_support_light = EffectPass::LT_DIRECTION;
							break;
                        case DEF_RENDERING_POINT_LIGHT:
                            this_pass.m_uniform_point = this_pass.m_shader->constant_buffer("PointLight");
							if (!this_pass.m_uniform_point) this_pass.m_uniform_point = this_pass.m_shader->constant_buffer("point_light");
							if (!this_pass.m_uniform_point) this_pass.m_uniform_spot = this_pass.m_shader->constant_buffer("light");
							this_pass.m_support_light = EffectPass::LT_POINT;
                            break;
                        case DEF_RENDERING_SPOT_LIGHT:
							this_pass.m_uniform_spot = this_pass.m_shader->constant_buffer("SpotLight");
							if (!this_pass.m_uniform_spot) this_pass.m_uniform_spot = this_pass.m_shader->constant_buffer("spot_light");
							if (!this_pass.m_uniform_spot) this_pass.m_uniform_spot = this_pass.m_shader->constant_buffer("light");
							this_pass.m_support_light = EffectPass::LT_SPOT;
                            break;
                        default:
							this_pass.m_support_light = EffectPass::LT_NONE;
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
                }// pass for each lights
			}
            
        }
        
        return true;
    }
    
}
}
