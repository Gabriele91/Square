//
//  ShaderVK.cpp
//  Square, the driver of Vulkan: the shaders (their SPIR-V in the layout of HLSL2ALL for
//  Vulkan: one set, one binding for each resource of the program), their uniforms
//
#include "RenderVK.h"
#include <algorithm>
#include <cstring>

namespace Square
{
namespace Render
{
	////////////////////////////////////////////////////////////////////////////////////////////
	// Uniforms
	void UniformVK::write(const void* data, size_t size)
	{
		//a loose uniform: its bytes in the global block (copied at the next draw)
		if (m_texture || !data) return;
		std::vector<unsigned char>& bytes = m_shader->m_global.m_bytes;
		if (m_offset >= bytes.size()) return;
		const size_t count = std::min(size, bytes.size() - m_offset);
		std::memcpy(bytes.data() + m_offset, data, count);
		m_shader->m_global.m_dirty = true;
	}

	void UniformVK::set(Texture* texture)
	{
		if (m_texture && m_binding < m_shader->m_textures.size())
		{
			m_shader->m_textures[m_binding] = texture;
		}
	}

	void UniformConstBufferVK::bind(const ConstBuffer* buffer)
	{
		if (m_binding < m_shader->m_buffers.size())
		{
			m_shader->m_buffers[m_binding] = buffer;
		}
	}

	void UniformConstBufferVK::unbind()
	{
		if (m_binding < m_shader->m_buffers.size())
		{
			m_shader->m_buffers[m_binding] = nullptr;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////
	// Shaders
	Shader* ContextVK::create_shader(const std::vector<ShaderSourceInformation>& infos)
	{
		static const VkShaderStageFlagBits stages_of[ST_N_SHADER] =
		{
			VK_SHADER_STAGE_VERTEX_BIT,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			VK_SHADER_STAGE_GEOMETRY_BIT,
			VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
			VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
			VK_SHADER_STAGE_COMPUTE_BIT
		};
		static const char* names[ST_N_SHADER] = { "VERTEX_SHADER", "FRAGMENT_SHADER", "GEOMETRY_SHADER", "TESS_CONTROL_SHADER", "TESS_EVALUATION_SHADER", "COMPUTE_SHADER" };
		auto* shader = new Shader();
		m_shaders.push_back(shader);
		std::vector<BindingVK> bindings;
		for (const ShaderSourceInformation& info : infos)
		{
			const int type = int(info.m_type);
			const std::string& bytes = info.m_shader_source;
			if (type < 0 || type >= ST_COMPUTE_SHADER || bytes.empty()) continue;
			if (bytes.size() % sizeof(uint32_t))
			{
				shader->m_errors.push_back(std::string(names[type]) + ": not SPIR-V (its size)");
				continue;
			}
			if (info.m_type == ST_GEOMETRY_SHADER && !m_features.geometryShader)
			{
				shader->m_errors.push_back(std::string(names[type]) + ": the device has no geometry shaders");
				continue;
			}
			const SpirvReflection::Stage stage = SpirvReflection::read(bytes);
			if (!stage.m_error.empty())
			{
				shader->m_errors.push_back(std::string(names[type]) + ": " + stage.m_error);
				continue;
			}
			if (stage.m_writes_layer && !m_vertex_layer)
			{
				shader->m_errors.push_back(std::string(names[type]) + ": the device cannot write the layer from the vertex shader");
				continue;
			}
			//its module (the words aligned)
			std::vector<uint32_t> words(bytes.size() / sizeof(uint32_t));
			std::memcpy(words.data(), bytes.data(), bytes.size());
			VkShaderModuleCreateInfo module{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
			module.codeSize = bytes.size();
			module.pCode = words.data();
			if (vkCreateShaderModule(m_device, &module, nullptr, &shader->m_modules[type]) != VK_SUCCESS)
			{
				shader->m_errors.push_back(std::string(names[type]) + ": unable to make its module");
				continue;
			}
			shader->m_entries[type] = info.m_entry_point;
			shader->m_writes_layer = shader->m_writes_layer || stage.m_writes_layer;
			if (info.m_type == ST_VERTEX_SHADER)
			{
				shader->m_inputs = stage.m_inputs;
			}
			//its resources into the bindings of the program (the same binding in each stage)
			for (const SpirvReflection::Resource& resource : stage.m_resources)
			{
				const bool uniform_buffer = resource.m_kind == SpirvReflection::ResourceKind::UNIFORM_BUFFER;
				const bool texture = resource.m_kind == SpirvReflection::ResourceKind::COMBINED_IMAGE_SAMPLER;
				if (!uniform_buffer && !texture)
				{
					shader->m_errors.push_back(std::string(names[type]) + ": " + resource.m_name + ", a resource of a kind not supported");
					continue;
				}
				if (resource.m_set != 0)
				{
					shader->m_errors.push_back(std::string(names[type]) + ": " + resource.m_name + ", not in the set 0 (the layout of HLSL2ALL for Vulkan)");
					continue;
				}
				auto it = std::find_if(bindings.begin(), bindings.end(), [&](const BindingVK& binding) { return binding.m_binding == resource.m_binding; });
				if (it == bindings.end())
				{
					BindingVK binding;
					binding.m_binding = resource.m_binding;
					binding.m_type = texture ? VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
					binding.m_name = resource.m_name;
					binding.m_size = resource.m_size;
					binding.m_global = resource.m_global;
					binding.m_dim = resource.m_dim;
					binding.m_arrayed = resource.m_arrayed;
					binding.m_count = resource.m_count;
					bindings.push_back(binding);
					it = bindings.end() - 1;
				}
				it->m_stages |= stages_of[type];
				it->m_size = std::max(it->m_size, resource.m_size);
			}
			//the loose uniforms (the members of the global block, of the first stage)
			if (shader->m_members.empty())
			{
				for (const SpirvReflection::Member& member : stage.m_members)
				{
					shader->m_members[member.m_name] = member;
				}
			}
		}
		std::sort(bindings.begin(), bindings.end(), [](const BindingVK& a, const BindingVK& b) { return a.m_binding < b.m_binding; });
		//its buffers: dynamic offsets (the same set, other slices of the ring), when the device has them
		uint32_t uniform_buffers = 0;
		for (const BindingVK& binding : bindings)
		{
			if (binding.m_type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER) uniform_buffers += 1;
		}
		const bool dynamic = uniform_buffers <= m_properties.limits.maxDescriptorSetUniformBuffersDynamic;
		uint32_t last = 0;
		std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
		for (BindingVK& binding : bindings)
		{
			if (binding.m_type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				binding.m_type = dynamic ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				shader->m_dynamic_buffers += dynamic ? 1 : 0;
			}
			if (binding.m_global)
			{
				shader->m_global_binding = int(binding.m_binding);
				shader->m_global.m_bytes.assign(binding.m_size, 0);
			}
			VkDescriptorSetLayoutBinding layout{};
			layout.binding = binding.m_binding;
			layout.descriptorType = binding.m_type;
			layout.descriptorCount = 1;
			layout.stageFlags = binding.m_stages;
			layout_bindings.push_back(layout);
			last = std::max(last, binding.m_binding + 1);
		}
		shader->m_bindings = std::move(bindings);
		shader->m_buffers.assign(last, nullptr);
		shader->m_textures.assign(last, nullptr);
		VkDescriptorSetLayoutCreateInfo set{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
		set.bindingCount = uint32_t(layout_bindings.size());
		set.pBindings = layout_bindings.data();
		if (vkCreateDescriptorSetLayout(m_device, &set, nullptr, &shader->m_set_layout) != VK_SUCCESS)
		{
			shader->m_link_error = "unable to make its descriptor set layout";
			return shader;
		}
		VkPipelineLayoutCreateInfo pipeline{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipeline.setLayoutCount = 1;
		pipeline.pSetLayouts = &shader->m_set_layout;
		if (vkCreatePipelineLayout(m_device, &pipeline, nullptr, &shader->m_pipeline_layout) != VK_SUCCESS)
		{
			shader->m_link_error = "unable to make its pipeline layout";
			return shader;
		}
		if (!shader->m_modules[ST_VERTEX_SHADER])
		{
			shader->m_link_error = "no vertex stage";
		}
		return shader;
	}

	bool ContextVK::shader_compiled_with_errors(Shader* shader)
	{
		return shader && !shader->m_errors.empty();
	}

	bool ContextVK::shader_linked_with_error(Shader* shader)
	{
		return !shader || !shader->m_link_error.empty();
	}

	std::vector<std::string> ContextVK::get_shader_compiler_errors(Shader* shader)
	{
		return shader ? shader->m_errors : std::vector<std::string>();
	}

	std::string ContextVK::get_shader_linker_error(Shader* shader)
	{
		return shader ? shader->m_link_error : std::string();
	}

	void ContextVK::bind_shader(Shader* shader)
	{
		m_shader = shader;
	}

	void ContextVK::unbind_shader(Shader* shader)
	{
		if (m_shader == shader) m_shader = nullptr;
	}

	void ContextVK::bind_uniform_CB(const ConstBuffer* buffer, Shader* shader, const std::string& uname)
	{
		if (auto* uniform = get_uniform_const_buffer(shader, uname))
		{
			uniform->bind(buffer);
		}
	}

	void ContextVK::unbind_uniform_CB(Shader* shader, const std::string& uname)
	{
		if (auto* uniform = get_uniform_const_buffer(shader, uname))
		{
			uniform->unbind();
		}
	}

	void ContextVK::forget_pipelines(const Shader* shader, const InputLayout* layout)
	{
		//the pipelines of a shader, of an input layout deleted (another one could have its address)
		std::vector<VkPipeline> pipelines;
		for (auto it = m_pipelines.begin(); it != m_pipelines.end();)
		{
			const bool of_it = (shader && it->first.m_shader == shader) || (layout && it->first.m_input_layout == layout);
			if (of_it)
			{
				if (it->second) pipelines.push_back(it->second);
				if (m_bound_pipeline == it->second) m_bound_pipeline = VK_NULL_HANDLE;
				it = m_pipelines.erase(it);
			}
			else
			{
				++it;
			}
		}
		if (pipelines.size())
		{
			VkDevice device = m_device;
			destroy_later([device, pipelines]()
			{
				for (VkPipeline pipeline : pipelines) vkDestroyPipeline(device, pipeline, nullptr);
			});
		}
	}

	void ContextVK::delete_shader(Shader*& shader)
	{
		if (!shader) return;
		if (m_shader == shader) m_shader = nullptr;
		forget_pipelines(shader, nullptr);
		m_shaders.erase(std::remove(m_shaders.begin(), m_shaders.end(), shader), m_shaders.end());
		VkDevice device = m_device;
		std::vector<VkShaderModule> modules;
		for (VkShaderModule module : shader->m_modules)
		{
			if (module) modules.push_back(module);
		}
		VkDescriptorSetLayout set_layout = shader->m_set_layout;
		VkPipelineLayout pipeline_layout = shader->m_pipeline_layout;
		destroy_later([device, modules, set_layout, pipeline_layout]()
		{
			for (VkShaderModule module : modules) vkDestroyShaderModule(device, module, nullptr);
			if (pipeline_layout) vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
			if (set_layout) vkDestroyDescriptorSetLayout(device, set_layout, nullptr);
		});
		delete shader;
		shader = nullptr;
	}

	Shader* ContextVK::get_bind_shader() const
	{
		return m_shader;
	}

	Uniform* ContextVK::get_uniform(Shader* shader, const std::string& uname) const
	{
		if (!shader) return nullptr;
		auto cached = shader->m_uniforms.find(uname);
		if (cached != shader->m_uniforms.end()) return cached->second.get();
		std::unique_ptr<UniformVK> uniform;
		//a texture (its binding)
		for (const BindingVK& binding : shader->m_bindings)
		{
			if (!uniform && binding.m_type == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER && binding.m_name == uname)
			{
				uniform = std::make_unique<UniformVK>(shader, binding.m_binding);
			}
		}
		//a loose uniform (its bytes in the global block)
		if (!uniform)
		{
			auto member = shader->m_members.find(uname);
			if (member == shader->m_members.end())
			{
				member = shader->m_members.find("_Global_" + uname);
			}
			if (member != shader->m_members.end())
			{
				uniform = std::make_unique<UniformVK>(shader, member->second.m_offset, member->second.m_size);
			}
		}
		if (!uniform) return nullptr;
		UniformVK* out = uniform.get();
		shader->m_uniforms[uname] = std::move(uniform);
		return out;
	}

	UniformConstBuffer* ContextVK::get_uniform_const_buffer(Shader* shader, const std::string& uname) const
	{
		if (!shader) return nullptr;
		auto cached = shader->m_uniform_buffers.find(uname);
		if (cached != shader->m_uniform_buffers.end()) return cached->second.get();
		for (const BindingVK& binding : shader->m_bindings)
		{
			const bool uniform_buffer = binding.m_type != VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			if (uniform_buffer && !binding.m_global && binding.m_name == uname)
			{
				auto uniform = std::make_unique<UniformConstBufferVK>(shader, binding.m_binding);
				UniformConstBufferVK* out = uniform.get();
				shader->m_uniform_buffers[uname] = std::move(uniform);
				return out;
			}
		}
		return nullptr;
	}
}
}
