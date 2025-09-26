#include "ConfigParser.hpp"

ConfigParser::ConfigParser(const std::string &file) : _file(file) {}

ServerConf ConfigParser::parse()
{
    std::ifstream ifs(_file.c_str());
    if (!ifs.is_open())
        throw std::runtime_error("Cannot open config file: " + _file);

    std::vector<std::string> tokens = tokenize(ifs);

    ServerConf conf;
    conf.client_max_body_size = 1000000; // 1MB
    conf.root = "/var/www/html";

    // ⚠️ Ici on fait simple : tu ajoutes ton parsing détaillé plus tard
    // Exemple: on force une écoute en 127.0.0.1:8080 si rien d’autre n’est trouvé
    conf.listen.push_back(std::make_pair("127.0.0.1", 8080));

    return conf;
}

std::vector<std::string> ConfigParser::tokenize(std::istream &ifs)
{
    std::vector<std::string> tokens;
    std::string line;
    while (std::getline(ifs, line))
	{
        std::istringstream iss(line);
        std::string tok;
        while (iss >> tok)
            tokens.push_back(tok);
    }
    return tokens;
}
