#pragma once
#include <SurfaceDataPBR>

#if defined( FORWARD_RENDERING )
#include <SurfaceForwardPBR>
#elif  defined( DEFERRED_RENDERING )
#include <SurfaceDeferredPBR>
#else
#error define type of rendering (FORWARD_RENDERING|DEFERRED_RENDERING)
#endif

#define surface SurfaceOutput fragment
#define surface_return return compute_surface_output
