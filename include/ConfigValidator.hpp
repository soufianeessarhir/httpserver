/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigValidator.hpp                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/21 20:39:43 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/21 20:39:50 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

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
