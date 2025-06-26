#include "MultiPart.hpp"

bool MultiPart::ProcessMultiPartHeaders(std::string& data)
{
    std::istringstream iss(data);
    std::string line;
    while (std::getline(iss,line))
    {
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        if (headers.size() == 0)
        {
            if (line.empty() || Request::OnlySpaces(line))
            {
                return false;
            }
        }
        if (line.empty())
        {
            return true;
        }
        size_t del = line.find(':');
        if(del ==  std::string::npos)
        {
            return false;
        }
        std::string name = line.substr(0,del);
        if (name.empty() || Request::Haswhitespace(name)) //[sessarhi] maybe i need to check for emply fileds | values
        {
            return false;
        }
        std::string value = line.substr(del + 1);
        Request::trim(value);
        Request::ToCanonical(name);
        headers[name] = value;
    }
    return true;
}