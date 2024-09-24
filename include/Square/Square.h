//
//  Square.h
//  Square
//
//  Created by Gabriele Di Bari on 18/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#pragma once
/* --- Config --- */
#include "Square/Config.h"
/* --- Core --- */
#include "Square/Core/Time.h"
#include "Square/Core/StringUtilities.h"
#include "Square/Core/UtfConvert.h"
#include "Square/Core/Filesystem.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Core/StringUtilities.h"
#include "Square/Core/ShellParser.h"
#include "Square/Core/Variant.h"
#include "Square/Core/Object.h"
#include "Square/Core/Resource.h"
#include "Square/Core/Attribute.h"
#include "Square/Core/ClassObjectRegistration.h"
#include "Square/Core/Context.h"
#include "Square/Core/Application.h"
/* --- Math --- */
#include "Square/Math/Linear.h"
#include "Square/Math/Tangent.h"
#include "Square/Math/Transformation.h"
/* --- Geometry --- */
#include "Square/Geometry/Sphere.h"
#include "Square/Geometry/Frustum.h"
#include "Square/Geometry/OBoundingBox.h"
#include "Square/Geometry/CreateBounding.h"
#include "Square/Geometry/AABoundingBox.h"
#include "Square/Geometry/Intersection.h"
/* --- Data --- */
#include "Square/Data/Json.h"
#include "Square/Data/Image.h"
#include "Square/Data/Archive.h"
#include "Square/Data/AttributeSerialize.h"
/* --- Parsers --- */
#include "Square/Data/ParserParameters.h"
#include "Square/Data/ParserResources.h"
#include "Square/Data/ParserEffect.h"
#include "Square/Data/ParserMaterial.h"
#include "Square/Data/ParserStaticMesh.h"
#include "Square/Data/ParserUtils.h"
/* --- Driver --- */
#include "Square/Driver/Render.h"
#include "Square/Driver/Input.h"
#include "Square/Driver/Window.h"
/* --- Render --- */
#include "Square/Render/Camera.h"
#include "Square/Render/Effect.h"
#include "Square/Render/GBuffer.h"
#include "Square/Render/Viewport.h"
#include "Square/Render/Queue.h"
#include "Square/Render/Light.h"
#include "Square/Render/Mesh.h"
#include "Square/Render/Transform.h"
#include "Square/Render/Renderable.h"
#include "Square/Render/VertexLayout.h"
#include "Square/Render/Drawer.h"
#include "Square/Render/DrawerPassDebug.h"
#include "Square/Render/DrawerPassForward.h"
#include "Square/Render/DrawerPassShadow.h"
/* --- Resource --- */
#include "Square/Resource/Texture.h"
#include "Square/Resource/Shader.h"
#include "Square/Resource/Effect.h"
#include "Square/Resource/Material.h"
#include "Square/Resource/Mesh.h"
#include "Square/Resource/Actor.h"
/* --- Scene --- */
#include "Square/Scene/Actor.h"
#include "Square/Scene/Component.h"
#include "Square/Scene/Level.h"
#include "Square/Scene/Camera.h"
#include "Square/Scene/DirectionLight.h"
#include "Square/Scene/PointLight.h"
#include "Square/Scene/SpotLight.h"
#include "Square/Scene/StaticMesh.h"
#include "Square/Scene/World.h"
/* main defines */
#include "Square/SquareMain.h"



