//
//  TextureDescriptor.h
//  AssetPack
//
//  A texture of the resources (.sqtex): its attributes (filters, wraps: lines kept as they are),
//  its image (url "x_img.dds", or its bytes after "data"), its fallback (an image for a GPU
//  without the format of the first one).
//
#pragma once
#include <Square/Square.h>

class TextureDescriptor
{
public:
    //a .sqtex (false: not a texture of url, image, texture or data)
    bool parse(const Square::Vector<unsigned char>& file);

    //the .sqtex: its attributes, its url, its fallback (if any)
    Square::Vector<unsigned char> serialize() const;

    Square::Vector<Square::String> m_attributes;   //the other lines (mag_filter linear...)
    Square::String                 m_url;          //the file of its image (empty: embedded)
    Square::Vector<unsigned char>  m_embedded;     //the bytes of its image ("data")
    Square::String                 m_fallback;     //the file of its fallback (empty: none)

private:
    //a line "key "value"": its key and its value (false: not one of them)
    static bool quoted(const Square::String& line, const char* key, Square::String& value);
    //a line of the text: the url, the fallback, else an attribute
    void parse_line(const Square::String& line);
};
