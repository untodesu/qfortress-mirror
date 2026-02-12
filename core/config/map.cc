#include "core/pch.hh"

#include "core/config/map.hh"

#include "core/config/value.hh"
#include "core/utils/physfs.hh"
#include "core/utils/string.hh"
#include "core/version.hh"

void ConfigMap::insert(ConfigValue& value) noexcept
{
    insert_or_assign(std::string(value.name()), &value);
}

void ConfigMap::load(std::istream& stream) noexcept
{
    std::string line;
    std::string kv_string;

    while(std::getline(stream, line)) {
        auto comment = line.find_first_of('#');

        if(comment == std::string::npos) {
            kv_string = utils::trim_whitespace<char>(line);
        }
        else {
            kv_string = utils::trim_whitespace<char>(line.substr(0, comment));
        }

        if(utils::is_whitespace<char>(kv_string)) {
            continue;
        }

        auto separator = kv_string.find('=');

        if(separator == std::string::npos) {
            LOG_WARNING("invalid line: '{}'", line);
            continue;
        }

        auto kv_name = kv_string.substr(0, separator);
        auto kv_value = kv_string.substr(separator + 1);

        kv_name = utils::trim_whitespace<char>(kv_name);
        kv_value = utils::trim_whitespace<char>(kv_value);

        auto kv_pair = this->find(kv_name);

        if(kv_pair == this->cend()) {
            LOG_WARNING("unknown key: {}", kv_name);
            continue;
        }

        if(!kv_pair->second->set_value(kv_value)) {
            LOG_WARNING("invalid value: key='{}' value='{}'", kv_name, kv_value);
            continue;
        }
    }
}

bool ConfigMap::load(std::string_view filename) noexcept
{
    std::istringstream stream;

    if(utils::read_file(filename, stream)) {
        load(stream);

        return true;
    }

    return false;
}

void ConfigMap::save(std::ostream& stream) const noexcept
{
    auto curtime = std::time(nullptr);

    stream << "# QFortress " << version::semantic << " configuration file" << std::endl;
    stream << "# Generated at: " << std::put_time(std::gmtime(&curtime), "%Y-%m-%d %H:%M:%S %z") << std::endl << std::endl;

    for(auto& it : (*this)) {
        stream << it.first << "=";
        stream << it.second->value();
        stream << std::endl;
    }
}

bool ConfigMap::save(std::string_view filename) const noexcept
{
    std::ostringstream stream;

    save(stream);

    return utils::write_file(filename, stream.str());
}
