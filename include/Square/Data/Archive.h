//
//  Square
//
//  Created by Gabriele on 09/09/16.
//  Copyright � 2016 Gabriele. All rights reserved.
//
#pragma once
#include "Square/Config.h"
#include "Square/Core/Uncopyable.h"
#include "Square/Core/SmartPointers.h"
#include "Square/Core/Variant.h"
#include <iostream>

namespace Square
{
//application instance class
class Application;
//contect instance class
class Context;
//data info
namespace  Data
{
	//The binary format (its version):
	//  0: no header, the attributes of an object one after another in their order (a new or a
	//     removed attribute shifted all the rest), the types of the compiler (long, long double)
	//  1: a header (magic "SQAR", the version of the format, of the engine); the attributes by
	//     name, each value in a block (one unknown, or not read: skipped); each component in a
	//     block (one unknown: skipped); the types of a size by any compiler (long: 64 bits,
	//     long double: a double)
	class SQUARE_API Archive
	{
	public:
		//the version of the format written
		static constexpr uint32 format_version = 1;
		//init archive
		Archive(Context& context, uint32 version = format_version) : m_context(context), m_version(version) {}
		//archive operator
		virtual Archive& operator % (VariantRef value) = 0;
		//a block: its size then its bytes (read: all of them, the reader can skip them)
		virtual Archive& block(std::string& data) = 0;
		//info
		Context& context() { return m_context;  }
		uint32 version() const { return m_version; }

	protected:
		//archive context
		Context & m_context;
		//the format of the stream
		uint32 m_version{ format_version };
	};

	class SQUARE_API ArchiveBinWrite : public Archive
	{
	public:
		//header: the magic, the versions of the format and of the engine at the start of the
		//stream (none: a block of another archive)
        ArchiveBinWrite(Context& context, std::ostream& stream, bool header = true);
		//Archivie operator
        virtual Archive& operator % (VariantRef value) override;
        virtual Archive& block(std::string& data) override;
	private:
		//output stream
		std::ostream& m_stream;
	};

    class SQUARE_API ArchiveBinRead : public Archive
    {
    public:
        //a stream: its header read (none: the format 0)
        ArchiveBinRead(Context& context, std::istream& stream);
        //a block of another archive (of its format, no header)
        ArchiveBinRead(Context& context, std::istream& stream, uint32 version);
        //Archivie operator
        virtual Archive& operator % (VariantRef value) override;
        virtual Archive& block(std::string& data) override;
        //the version of the engine that wrote it ("" without a header)
        const std::string& engine_version() const { return m_engine_version; }
    private:
        //output stream
        std::istream& m_stream;
        std::string   m_engine_version;
    };


}
}
