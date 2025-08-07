


#ifndef CONFIGVALIDATOR_HPP
#define CONFIGVALIDATOR_HPP
#include <ConfigData.hpp>
class ConfigValidator
{
    std::vector<Server> &servers;
    public:
    ConfigValidator(std::vector<Server>&srv);
    static void ValidateConfig();
};

#endif