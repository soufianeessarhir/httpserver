#ifndef POST_HPP
#define POST_HPP

#include <fstream>
class Post
{
public:
    Post(std::string &);
    ~Post();
    void ProcessChunck();
    void ProcessContentLength();
    bool IsChuck()const;
    void SetChunked(bool);
private:
    std::ofstream OutputFile;
    bool is_chunk;
};
#endif