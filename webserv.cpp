/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   webserv.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sessarhi <sessarhi@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/18 15:26:08 by sessarhi          #+#    #+#             */
/*   Updated: 2025/08/23 19:57:08 by sessarhi         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "webserv.hpp"

int main(int argc, char **argv)
{
	if (argc != 2)
		return std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl, 1;

    signal(SIGPIPE, SIG_IGN);;

	std::vector<Server> servers;
	std::ifstream configFile(argv[1]);
	if (!configFile.is_open())
		return std::cerr << "Error: Could not open config file." << std::endl, 1;
	Parser parser(configFile, servers);
	try
	{
		parser.Config();
		if (servers.empty())
		{
			std::cerr << "Error: No valid server configurations found." << std::endl;
			return 1;
		}
		HttpServer httpServer(servers);
		httpServer.run();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return  1;
	}
	return 0;
}