#include "Post.hpp"

Post::Post(std::string & filename):OutputFile(filename.c_str())
{

}

void Post::ProcessChunck()
{

}

void Post::ProcessContentLength()
{

}

bool Post::IsChuck()const
{
    return is_chunk;
}
Post::~Post()
{
    if (OutputFile.is_open())
    {
        OutputFile.close();
    }
}