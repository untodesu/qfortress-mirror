#ifndef CORE_ENTITY_COMPONENTS_HH
#define CORE_ENTITY_COMPONENTS_HH
#pragma once

class ReadBuffer;
class WriteBuffer;

namespace components
{
using serialize_fn = std::function<JSON_Value*(entt::entity entity)>;
using deserialize_fn = std::function<void(entt::entity entity, const JSON_Value* jsonv)>;
} // namespace components

namespace components
{
void register_component(std::string_view name, serialize_fn serializer, deserialize_fn deserializer);
} // namespace components

namespace components
{
JSON_Value* serialize_entity(entt::entity entity);
void deserialize_entity(entt::entity entity, const JSON_Value* jsonv);
} // namespace components

#endif
