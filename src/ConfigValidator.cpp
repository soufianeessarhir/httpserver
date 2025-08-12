#include "ConfigValidator.hpp"
#include <algorithm>

ConfigValidator::ConfigValidator(std::vector<Server>& srv) : servers(srv) {}

void ConfigValidator::CheckSharedInterface(Server &fr, Server &sc)
{
    for (size_t i = 0; i < fr.listen.size(); ++i)
    {
        for (size_t j = 0; j < sc.listen.size(); ++j)
        {
            if (&fr == &sc && j <= i)
                continue;
            bool samePort = (fr.listen[i].second == sc.listen[j].second);
            bool sameIP   = (fr.listen[i].first == sc.listen[j].first);
            bool wildcardConflict = (fr.listen[i].first == "0.0.0.0" || sc.listen[j].first == "0.0.0.0");
            if (samePort && (sameIP || wildcardConflict))
            {
                for (size_t k = 0; k < fr.server_names.size(); ++k)
                    if (std::find(sc.server_names.begin(), sc.server_names.end(),
                                  fr.server_names[k]) != sc.server_names.end())
                        throw ParseException("ambiguis configuration");
                if (!sc.isvirtual)
                    sc.isvirtual = true;
                sc.virtual_listen.push_back(sc.listen[j]);
            }
        }
    }
}

void ConfigValidator::CkeckRoot(Server &srv)
{
    bool has_no_root = srv.root.empty();
    for (std::map<std::string, LocationData>::iterator it = srv.locations.begin();
         it != srv.locations.end(); ++it)
        if (has_no_root && (*it).second.root.empty())
            throw ParseException("location should define its root");
}

void ConfigValidator::CheckListenDup(Server &srv)
{
    bool samePort;
    bool sameIP;
    bool wildcardConflict;
    if (srv.listen.empty())
        throw ParseException("no interface specified for binding");
    for (size_t i = 0; i < srv.listen.size(); ++i)
    {
        for (size_t j = i + 1; j < srv.listen.size(); ++j)
        {
            samePort = (srv.listen[i].second == srv.listen[j].second);
            sameIP   = (srv.listen[i].first == srv.listen[j].first);
            wildcardConflict = (srv.listen[i].first == "0.0.0.0" || 
                                srv.listen[j].first == "0.0.0.0");
            if (samePort && (sameIP || wildcardConflict))
                throw ParseException("cannot bind to the same interface");
        }
    }
}

void ConfigValidator::ValidateConfig()
{
    for (size_t i = 0 ; i < servers.size(); ++i)
    {
        if (servers[i].locations.empty())
            throw ParseException("server has no locations");
        CheckListenDup(servers[i]);
        for (size_t j = i + 1; j < servers.size(); ++j)
        {
            CheckSharedInterface(servers[i], servers[j]);
        }
    }
}