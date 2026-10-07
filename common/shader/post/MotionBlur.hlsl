//
//  MotionBlur.hlsl
//  Square
//
//  The motion blur of the objects (see PostEffectMotionBlur): the velocity of the frame (uv on
//  the screen, 0 where nothing has its motion blur) at the pixel and at a few taps around it,
//  the longest one taken (the blur goes a little out of the silhouette), the pixel blurred along
//  it (a share of it, at most some pixels).
//
#include <Camera>
#include <Vertex>
#include <DeferredFullscreen>

Sampler2D(g_source);
Sampler2D(g_velocity);
Vec2 mb_size;   //pixels of the frame
Vec4 mb_params; //shutter, max pixels, min pixels, samples

//the motion of a pixel in pixels (the shutter, at most max pixels)
Vec2 mb_motion(in Vec2 uv)
{
	Vec2  motion = texture2DLod(g_velocity, uv, 0.0).xy * mb_size * mb_params.x;
	float pixels = length(motion);
	if (pixels <= mb_params.y) return motion;
	return motion * (mb_params.y / pixels);
}

Vec4 fragment(DeferredVSOutput input) : SV_TARGET0
{
	Vec2  uv = input.m_position.xy / mb_size;
	Vec2  texel = 1.0 / mb_size;
	Vec3  center = texture2DLod(g_source, uv, 0.0).rgb;
	//the longest motion around (the pixel, eight taps at half the blur at most)
	Vec2  motion = mb_motion(uv);
	float best = dot(motion, motion);
	float reach = mb_params.y * 0.5;
	for (int k = 0; k < 8; ++k)
	{
		float a = float(k) * 0.78539816;
		Vec2  near_motion = mb_motion(uv + Vec2(cos(a), sin(a)) * reach * texel);
		float near_length = dot(near_motion, near_motion);
		if (near_length > best)
		{
			best = near_length;
			motion = near_motion;
		}
	}
	if (sqrt(best) < mb_params.z) return Vec4(center, 1.0);
	//the samples along the motion, centered on the pixel
	int   samples = int(mb_params.w);
	Vec2  step = motion * texel / float(samples - 1);
	Vec2  start = uv - step * (float(samples - 1) * 0.5);
	Vec3  sum = Vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < 32; ++i)
	{
		if (i >= samples) break;
		sum += texture2DLod(g_source, start + step * float(i), 0.0).rgb;
	}
	return Vec4(sum / float(samples), 1.0);
}
