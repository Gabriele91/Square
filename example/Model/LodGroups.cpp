//
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "LodGroups.h"
#include "SquareExtras.h"
#include <cctype>
#include <algorithm>
#include <map>
#include <vector>

namespace LodGroups
{
    namespace AuxLodGroups
    {
        //the levels of a name "<base>_lod<n>" (n: one or two digits): its base, its level
        static bool level_name(const std::string& name, std::string& base, size_t& level)
        {
            const size_t at = name.rfind("_lod");
            if (at == std::string::npos || at == 0) return false;
            const std::string digits = name.substr(at + 4);
            if (digits.empty() || digits.size() > 2) return false;
            for (const char c : digits)
            {
                if (!std::isdigit((unsigned char)c)) return false;
            }
            base = name.substr(0, at);
            level = size_t(std::stoul(digits));
            return true;
        }

        //the defaults of a mode for some levels: screen 0.6, 0.3, 0.15... the last 0.1 (culled
        //under a tenth of the screen, as Unity); distance 100, 200, 400...
        static std::vector<float> defaults(Square::Scene::LodMode mode, size_t count)
        {
            std::vector<float> thresholds;
            thresholds.reserve(count);
            for (size_t i = 0; i != count; ++i)
            {
                const float doubled = float(size_t(1) << i);
                const bool  last = i + 1 == count;
                switch (mode)
                {
                case Square::Scene::LodMode::DISTANCE:
                    thresholds.push_back(100.0f * doubled);
                    break;
                case Square::Scene::LodMode::SCREEN:
                default:
                {
                    const float halved = 0.6f / doubled;
                    thresholds.push_back(last ? std::min(0.1f, halved) : halved);
                }
                break;
                }
            }
            return thresholds;
        }

        //the extras of the first level that has them
        static const Square::Data::JsonObject* first_extras(const std::map< size_t, Square::Shared<Square::Scene::Actor> >& levels, const Extras& extras)
        {
            for (const auto& level : levels)
            {
                auto it = extras.find(level.second.get());
                if (it != extras.end()) return &it->second;
            }
            return nullptr;
        }
    }

    size_t build(Square::Context& context, const Square::Shared<Square::Scene::Actor>& root, const Extras& extras)
    {
        using namespace Square;
        using namespace Square::Scene;
        //the nodes (before the groups: the levels moved under them are not grouped again)
        std::vector< Shared<Actor> > parents;
        root->visit([&parents](Shared<Actor> node) -> bool
        {
            parents.push_back(node);
            return true;
        });
        size_t groups = 0;
        for (const auto& parent : parents)
        {
            //its children by their base: their levels
            std::map< std::string, std::map< size_t, Shared<Actor> > > sets;
            for (const auto& child : parent->childs())
            {
                std::string base;
                size_t level = 0;
                if (AuxLodGroups::level_name(child->name(), base, level))
                {
                    sets[base][level] = child;
                }
            }
            for (const auto& set : sets)
            {
                const auto& levels = set.second;
                //its mode, its thresholds
                LodMode mode = LodMode::SCREEN;
                std::vector<float> thresholds;
                if (const Data::JsonObject* properties = AuxLodGroups::first_extras(levels, extras))
                {
                    const auto mode_name = SquareExtras::string(*properties, "lod_mode");
                    if (mode_name && *mode_name == "distance")
                    {
                        mode = LodMode::DISTANCE;
                    }
                    auto values = properties->find(SquareExtras::PREFIX + "lod");
                    if (values != properties->end())
                    {
                        for (size_t i = 0; i != levels.size(); ++i)
                        {
                            thresholds.push_back(float(SquareExtras::component(values->second, i)));
                        }
                    }
                }
                if (thresholds.empty())
                {
                    thresholds = AuxLodGroups::defaults(mode, levels.size());
                }
                //the group: a node of the base, the levels under it (in their place: it is where
                //their parent is)
                auto group_node = MakeShared<Actor>(context, set.first);
                parent->add(group_node);
                std::vector<LodGroup::Level> group_levels;
                group_levels.reserve(levels.size());
                for (const auto& level : levels)
                {
                    group_node->add(level.second);
                    const float threshold = thresholds[group_levels.size()];
                    group_levels.push_back({ level.second->name(), threshold });
                }
                auto group = group_node->component<LodGroup>();
                group->mode(mode);
                group->levels(group_levels);
                if (!group->recalculate_bounds())
                {
                    context.logger()->warning("Level of detail without meshes: " + set.first);
                }
                ++groups;
            }
        }
        return groups;
    }
}
