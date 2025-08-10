#include "ConfigValidator.hpp"

ConfigValidator::ConfigValidator(std::vector<Server>&srv):servers(srv)
{


}
void ConfigValidator::CheckSharedInterface(Server &fr, Server &sc)
{

    for (size_t i = 0; i < fr.listen.size(); ++i)
    {
        for (size_t j = i + 1; j < sc.listen.size(); ++j)
        {
            bool samePort = (fr.listen[i].second == sc.listen[j].second);
            bool sameIP   = (fr.listen[i].first == sc.listen[j].first);
            bool wildcardConflict = (fr.listen[i].first == "0.0.0.0" || 
                    sc.listen[j].first == "0.0.0.0");
            if (samePort && (sameIP || wildcardConflict))
            {
                for (size_t i = 0;i <fr.server_names.size();++i)
                {
                    if (std::find(sc.server_names.begin(),sc.server_names.end(),fr.server_names[i]) != sc.server_names.end())
                        throw ParseException("ambiguis configuration");
                }
                if (! sc.isvirtual)
                    sc.isvirtual = true;
                sc.virtual_listen.push_back(std::make_pair(sc.listen[j].first,sc.listen[j].second));
            }
        }
    }
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
    for (size_t i = 0 ; i < servers.size();++i)
	{
		CheckListenDup(servers[i]);
        for (size_t j = i ; j < servers.size();++j)
        {
            CheckSharedInterface(servers[i],servers[j]);
        }
	}
}

