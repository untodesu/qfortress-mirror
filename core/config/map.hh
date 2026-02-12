#ifndef CORE_CONFIG_MAP_HH
#define CORE_CONFIG_MAP_HH
#pragma once

class ConfigValue;

class ConfigMap final : public std::unordered_map<std::string, ConfigValue*> {
public:
    ConfigMap(void) = default;
    virtual ~ConfigMap(void) = default;

    ConfigMap(const ConfigMap&) = delete;
    ConfigMap& operator=(const ConfigMap&) = delete;

    void insert(ConfigValue& value) noexcept;

    void load(std::istream& stream) noexcept;
    bool load(std::string_view filename) noexcept;

    void save(std::ostream& stream) const noexcept;
    bool save(std::string_view filename) const noexcept;
};

#endif
