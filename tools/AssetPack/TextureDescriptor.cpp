//
//  TextureDescriptor.cpp
//  AssetPack
//
//  A texture of the resources (.sqtex): its attributes (filters, wraps: lines kept as they are),
//  its image (url "x_img.dds", or its bytes after "data"), its fallback (an image for a GPU
//  without the format of the first one).
//
#include "TextureDescriptor.h"
#include <cstring>

//the keys of the image (the same one: Data/ParserTexture.cpp)
static const char* s_image_keys[] = { "url", "image", "texture" };
//the bytes of the image follow it at once, to the end of the file
static const char  s_data_key[] = "data";

bool TextureDescriptor::quoted(const Square::String& line, const char* key, Square::String& value)
{
    const size_t key_size = std::strlen(key);
    if (line.compare(0, key_size, key) != 0)
    {
        return false;
    }
    const size_t first = line.find('"', key_size);
    if (first == Square::String::npos)
    {
        return false;
    }
    const size_t last = line.find('"', first + 1);
    if (last == Square::String::npos)
    {
        return false;
    }
    value = line.substr(first + 1, last - first - 1);
    return true;
}

void TextureDescriptor::parse_line(const Square::String& line)
{
    for (const char* key : s_image_keys)
    {
        if (quoted(line, key, m_url))
        {
            return;
        }
    }
    if (quoted(line, "fallback", m_fallback))
    {
        return;
    }
    if (!line.empty())
    {
        m_attributes.push_back(line);
    }
}

bool TextureDescriptor::parse(const Square::Vector<unsigned char>& file)
{
    m_attributes.clear();
    m_url.clear();
    m_embedded.clear();
    m_fallback.clear();
    size_t line_start = 0;
    while (line_start < file.size())
    {
        //"data": the bytes of the image to the end
        const size_t data_size = sizeof(s_data_key) - 1;
        if (file.size() - line_start >= data_size && std::memcmp(&file[line_start], s_data_key, data_size) == 0)
        {
            m_embedded.assign(file.begin() + line_start + data_size, file.end());
            break;
        }
        size_t line_end = line_start;
        while (line_end < file.size() && file[line_end] != '\n')
        {
            ++line_end;
        }
        Square::String line(file.begin() + line_start, file.begin() + line_end);
        if (!line.empty() && line.back() == '\r')
        {
            line.pop_back();
        }
        parse_line(line);
        line_start = line_end + 1;
    }
    return !m_url.empty() || !m_embedded.empty();
}

Square::Vector<unsigned char> TextureDescriptor::serialize() const
{
    Square::String text;
    for (const Square::String& attribute : m_attributes)
    {
        text += attribute + "\n";
    }
    if (!m_url.empty())
    {
        text += "url \"" + m_url + "\"\n";
    }
    if (!m_fallback.empty())
    {
        text += "fallback \"" + m_fallback + "\"\n";
    }
    Square::Vector<unsigned char> out(text.begin(), text.end());
    if (m_url.empty() && !m_embedded.empty())
    {
        out.insert(out.end(), s_data_key, s_data_key + sizeof(s_data_key) - 1);
        out.insert(out.end(), m_embedded.begin(), m_embedded.end());
    }
    return out;
}
