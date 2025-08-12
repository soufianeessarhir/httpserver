


#ifndef CONFIGVALIDATOR_HPP
#define CONFIGVALIDATOR_HPP
#include "ConfigData.hpp"
#include "Exceptions.hpp"
#include <algorithm>
class ConfigValidator
{
    std::vector<Server> &servers;
    public:
    void CkeckRoot(Server &srv);
    ConfigValidator(std::vector<Server>&srv);
    void CheckSharedInterface(Server &fr, Server &sc);
    void CheckListenDup(Server &srv);
    void ValidateConfig();
};

#endif
