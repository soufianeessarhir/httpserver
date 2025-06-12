#ifndef POST_HPP
#define POST_HPP

#include <fstream>
class Post
{
public:
    Post(std::string &);
    void ProcessChunck();
    void ProcessContentLength();
    bool IsChuck()const;
    ~Post();
private:
    std::ofstream OutputFile;
    bool is_chunk;
};
#endif