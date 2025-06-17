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

Post::~Post()
{
    if (OutputFile.is_open())
    {
        OutputFile.close();
    }
}