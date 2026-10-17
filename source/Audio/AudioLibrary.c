//
//  AudioLibrary.c
//  Square
//
//  Created by Gabriele Di Bari on 17/10/26.
//  Copyright © 2026 Gabriele Di Bari. All rights reserved.
//
//  The implementation of miniaudio, with stb_vorbis for the .ogg files
//  (the header of stb_vorbis before miniaudio, its implementation after).
//
#define STB_VORBIS_HEADER_ONLY
#include "extras/stb_vorbis.c"
#include "miniaudio.c"
#undef STB_VORBIS_HEADER_ONLY
#include "extras/stb_vorbis.c"
