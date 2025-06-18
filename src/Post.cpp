#include "Post.hpp"

Post::Post(std::string & filename):output_file(filename.c_str())
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
    if (output_file.is_open())
    {
        output_file.close();
    }
}