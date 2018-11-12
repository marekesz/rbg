//
// Created by ??? on 12.01.18.
//

#ifndef RBGGAMEMANAGER_NAME_RESOLVER_H
#define RBGGAMEMANAGER_NAME_RESOLVER_H

#include <vector>
#include <unordered_map>

using token_id_t = std::size_t;

/* TODO(???): Probably go back to the split version of NameResolver. It is handling too much right now. */
class NameResolver {
public:
  NameResolver() = default;

  void AddName(const std::string &name) {
    names_.push_back(name);
    names_ids_[name] = names_.size() - 1;
  }

  token_id_t Id(const std::string &name) const {
    return names_ids_.at(name);
  }

  const std::string &Name(token_id_t id) const {
    return names_[id];
  }

  size_t NamesCount() const {
    return names_.size();
  }

private:
  std::vector<std::string> names_;
  std::unordered_map<std::string, token_id_t> names_ids_;
};


#endif //RBGGAMEMANAGER_NAME_RESOLVER_H
