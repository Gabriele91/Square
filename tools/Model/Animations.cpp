//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Animations.h"
#include "GLTFImport.h"
#include "MeshManager.h"
#include <algorithm>

namespace Animations
{
    namespace AuxAnimations
    {
        using namespace Square;
        using namespace Square::Scene;
        using namespace Square::Data;

        //a key of a channel in the axes of the scene (a tangent of a cubic key too: the turn is linear)
        Vec4 turned(Animator::Property property, const Vec4& value, unsigned char mode)
        {
            Vec3 translation(0.0f);
            Quat rotation(0.0f, 0.0f, 0.0f, 1.0f);
            Vec3 scale(1.0f);
            Vec4 out = value;
            switch (property)
            {
            case Animator::P_ROTATION:
            {
                rotation = Quat(value.x, value.y, value.z, value.w);
                turn(translation, rotation, scale, mode);
                out = Vec4(rotation.x, rotation.y, rotation.z, rotation.w);
            }
            break;
            case Animator::P_SCALE:
            {
                scale = Vec3(value);
                turn(translation, rotation, scale, mode);
                out = Vec4(scale, 0.0f);
            }
            break;
            default:
            case Animator::P_POSITION:
            {
                translation = Vec3(value);
                turn(translation, rotation, scale, mode);
                out = Vec4(translation, 0.0f);
            }
            break;
            }
            return out;
        }

        //a channel of glTF as a channel of the animator (false: not taken)
        bool channel_of
        (
              const GLTF::GLTF& gltf
            , const GLTF::Animation& animation
            , const GLTF::Animation::Channel& channel
            , unsigned char mode
            , Animator::Channel& out
        )
        {
            const bool taken = channel.path != GLTF::Animation::Path::WEIGHTS && channel.sampler < animation.samplers.size();
            if (taken)
            {
                const GLTF::Animation::Sampler& sampler = animation.samplers[channel.sampler];
                switch (channel.path)
                {
                case GLTF::Animation::Path::ROTATION: out.m_property = Animator::P_ROTATION; break;
                case GLTF::Animation::Path::SCALE:    out.m_property = Animator::P_SCALE; break;
                default:                              out.m_property = Animator::P_POSITION; break;
                }
                switch (sampler.interpolation)
                {
                case GLTF::Animation::Interpolation::STEP:        out.m_interpolation = Animator::I_STEP; break;
                case GLTF::Animation::Interpolation::CUBICSPLINE: out.m_interpolation = Animator::I_CUBIC; break;
                default:                                          out.m_interpolation = Animator::I_LINEAR; break;
                }
                out.m_times = GLTF::Import::read_floats(gltf, sampler.input, false);
                //the values: 3 or 4 components (rotations in bytes or shorts: normalized)
                const std::vector<float> values = GLTF::Import::read_floats(gltf, sampler.output, true);
                const size_t components = out.m_property == Animator::P_ROTATION ? 4 : 3;
                out.m_values.reserve(values.size() / components);
                for (size_t i = 0; i + components <= values.size(); i += components)
                {
                    Vec4 value(values[i], values[i + 1], values[i + 2], components == 4 ? values[i + 3] : 0.0f);
                    out.m_values.push_back(turned(out.m_property, value, mode));
                }
            }
            return taken && !out.m_times.empty();
        }
    }

    void turn(Square::Vec3& translation, Square::Quat& rotation, Square::Vec3& scale, unsigned char mode)
    {
        using namespace Square;
        if (mode & M_SWAP_ZY)
        {
            // Swap Y Z
            const Quat swap_zy = angle_axis(radians(90.0f), Constants::axis_x);
            std::swap(translation.z, translation.y);
            rotation = swap_zy * rotation * conjugate(swap_zy);
            std::swap(scale.z, scale.y);
        }
        if (mode & M_TO_LHS)
        {
            translation.z = translation.z != 0.0f ? -translation.z : translation.z;
            rotation = Quat(rotation.x, rotation.y, -rotation.z, -rotation.w);
        }
    }

    bool path(const Square::Scene::Actor& root, const Square::Scene::Actor& actor, std::string& out)
    {
        using namespace Square::Scene;
        std::vector<std::string> names;
        const Actor* at = &actor;
        Square::Shared<Actor> parent;
        while (at && at != &root)
        {
            names.push_back(at->name());
            parent = at->parent().lock();
            at = parent.get();
        }
        const bool under = at == &root;
        out.clear();
        for (auto name = names.rbegin(); under && name != names.rend(); ++name)
        {
            out += (out.empty() ? "" : "/") + *name;
        }
        return under;
    }

    size_t build
    (
          Square::Context& context
        , const Square::Shared<Square::Scene::Actor>& root
        , const Square::Data::GLTF::GLTF& gltf
        , const NodeActors& actors
        , unsigned char mode
    )
    {
        using namespace AuxAnimations;
        std::vector<Animator::Clip> clips;
        for (const GLTF::Animation& animation : gltf.animations)
        {
            Animator::Clip clip;
            clip.m_name = animation.name.size() ? animation.name : "clip_" + std::to_string(clips.size());
            for (const GLTF::Animation::Channel& channel : animation.channels)
            {
                Shared<Actor> target;
                if (channel.node.has_value() && *channel.node < actors.size())
                {
                    target = actors[*channel.node];
                }
                Animator::Channel out;
                std::string target_path;
                const bool found = target && path(*root, *target, target_path);
                if (found && channel_of(gltf, animation, channel, mode, out))
                {
                    out.m_target = target_path;
                    clip.m_duration = std::max(clip.m_duration, out.m_times.back());
                    clip.m_channels.push_back(std::move(out));
                    //it moves
                    target->set_static(false);
                }
                else if (!found && channel.path != GLTF::Animation::Path::WEIGHTS)
                {
                    context.logger()->warning("Animation " + clip.m_name + ": a channel of a node not in the scene");
                }
            }
            if (!clip.m_channels.empty())
            {
                //from its first key (Blender: frame 1 at 1/fps), its loop without a still frame
                float start = clip.m_duration;
                for (const auto& channel : clip.m_channels)
                {
                    start = std::min(start, channel.m_times.front());
                }
                for (auto& channel : clip.m_channels)
                {
                    for (float& time : channel.m_times)
                    {
                        time -= start;
                    }
                }
                clip.m_duration -= start;
                clips.push_back(std::move(clip));
            }
        }
        if (!clips.empty())
        {
            auto animator = root->component<Animator>();
            animator->clips(clips);
            animator->autoplay(true);
        }
        return clips.size();
    }
}
