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
	class SQUARE_API Archive
	{
	public:
		//init archivie
		Archive(Context& context) : m_context(context) {}
		//archivie operator
		virtual Archive& operator % (VariantRef value) = 0;
		//info
		Context& context() { return m_context;  }

	protected:
		//archivie context
		Context & m_context;
	};

	class SQUARE_API ArchiveBinWrite : public Archive
	{
	public:
		//Input
        ArchiveBinWrite(Context& context, std::ostream& stream);
		//Archivie operator
        virtual Archive& operator % (VariantRef value);
	private:
		//output stream
		std::ostream& m_stream;
	};
    
    class SQUARE_API ArchiveBinRead : public Archive
    {
    public:
        //Input
        ArchiveBinRead(Context& context, std::istream& stream);
        //Archivie operator
        virtual Archive& operator % (VariantRef value);
    private:
        //output stream
        std::istream& m_stream;
    };
}
}
