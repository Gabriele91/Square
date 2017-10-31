//
//  Screen_private.m
//  Square
//
//  Created by Gabriele Di Bari on 30/10/17.
//  Copyright © 2017 Gabriele Di Bari. All rights reserved.
//
#include "Screen_private.h"

namespace Square
{
namespace Video
{
namespace Cocoa
{

    ScreenCocoa::ScreenCocoa()
    {
    }
    
    ScreenCocoa::ScreenCocoa(uint32_t id, uint32_t unit_number, const CGSize size)
    {
        m_id = id;
        m_unit_number = unit_number;
        m_size[0] = size.width;
        m_size[1] = size.height;
    }
    
    void ScreenCocoa::get_position(unsigned int size[2])
    {
        const CGRect bounds = CGDisplayBounds(m_id);
        size[0] =  bounds.origin.x;
        size[1] =  bounds.origin.x;
    }
    
    size_t ScreenCocoa::display_bits_per_pixel_for_mode(CGDisplayModeRef mode)
    {
        size_t depth = 0;
        CFStringRef pix_enc = CGDisplayModeCopyPixelEncoding (mode);
        
        if(CFStringCompare(pix_enc, CFSTR(IO32BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
            depth = 32;
        else if(CFStringCompare(pix_enc, CFSTR(IO16BitDirectPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
            depth = 16;
        else if(CFStringCompare(pix_enc, CFSTR(IO8BitIndexedPixels), kCFCompareCaseInsensitive) == kCFCompareEqualTo)
            depth = 8;
        
        return depth;
    }
    
    CGDisplayModeRef ScreenCocoa::best_match_for_mode( ScreenMode screenMode )
    {
        
        bool exactMatch = false;
        
        // Get a copy of the current display mode
        CGDisplayModeRef displayMode = CGDisplayCopyDisplayMode(m_id);
        
        // Loop through all display modes to determine the closest match.
        // CGDisplayBestModeForParameters is deprecated on 10.6 so we will emulate it's behavior
        // Try to find a mode with the requested depth and equal or greater dimensions first.
        // If no match is found, try to find a mode with greater depth and same or greater dimensions.
        // If still no match is found, just use the current mode.
        CFArrayRef allModes = CGDisplayCopyAllDisplayModes(kCGDirectMainDisplay, NULL);
        for(int i = 0; i < CFArrayGetCount(allModes); i++)
        {
            CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);
            
            if(display_bits_per_pixel_for_mode( mode ) != screenMode.depth)
                continue;
            
            if((CGDisplayModeGetWidth(mode) >= screenMode.width) &&
               (CGDisplayModeGetHeight(mode) >= screenMode.height) &&
               (CGDisplayModeGetWidth(displayMode) >= CGDisplayModeGetWidth(mode))&&
               (CGDisplayModeGetHeight(displayMode) >= CGDisplayModeGetHeight(mode)))
            {
                displayMode = mode;
                exactMatch = true;
            }
        }
        
        // No depth match was found
        if(!exactMatch)
        {
            for(int i = 0; i < CFArrayGetCount(allModes); i++)
            {
                CGDisplayModeRef mode = (CGDisplayModeRef)CFArrayGetValueAtIndex(allModes, i);
                if(display_bits_per_pixel_for_mode( mode )  >= screenMode.depth)
                    continue;
                
                
                if((CGDisplayModeGetWidth(mode) >= screenMode.width) &&
                   (CGDisplayModeGetHeight(mode) >= screenMode.height) &&
                   (CGDisplayModeGetWidth(displayMode) >= CGDisplayModeGetWidth(mode))&&
                   (CGDisplayModeGetHeight(displayMode) >= CGDisplayModeGetHeight(mode)))
                {
                    displayMode = mode;
                }
            }
        }
        
        return displayMode;
    }
}
}
}
