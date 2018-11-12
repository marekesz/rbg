//
// Created by ??? on 13.05.18.
//

#ifndef RBGGAMEMANAGER_GRAPH_BOARD_H
#define RBGGAMEMANAGER_GRAPH_BOARD_H

#include <vector>
#include <unordered_map>
#include "name_resolver.h"

using vertex_t = ssize_t;
using edge_name_t = size_t;

class EdgeResolver
{
public:
  EdgeResolver() = default;

  void AddEdge(const std::string &name) {
    names_.push_back(name);
    names_ids_[name] = names_.size() - 1;
  }

  edge_name_t Id(const std::string &name) const {
    return names_ids_.at(name);
  }

  const std::string &Name(edge_name_t id) const {
    return names_[id];
  }

  size_t EdgesCount() const {
    return names_.size();
  }

private:
  std::vector<std::string> names_;
  std::unordered_map<std::string, edge_name_t> names_ids_;
};

class VertexResolver
{
  public:
  VertexResolver(std::size_t vertices_count)
      : names_(vertices_count)
  {}

  void SetName(vertex_t index, const std::string &name) {
    names_[index] = name;
    names_ids_[name] = index;
  }

  vertex_t Id(const std::string &name) const {
    return names_ids_.at(name);
  }

  const std::string &Name(vertex_t id) const {
    return names_[id];
  }

  size_t VerticesCount() const {
    return names_.size();
  }

  private:
  std::vector<std::string> names_;
  std::unordered_map<std::string, vertex_t> names_ids_;
};

class GraphBoard
{
public:
  explicit GraphBoard(size_t graph_size)
      : neighbours_(graph_size), fields_(graph_size)
  {}

  void AddEdge(vertex_t a, edge_name_t edge_name, vertex_t b)
  {
    neighbours_[a][edge_name] = b;
  }

  vertex_t Next(vertex_t a, edge_name_t edge_name) const
  {
    auto it = neighbours_[a].find(edge_name);
    if(it != neighbours_[a].end())
      return it->second;
    else
      return -1;
  }

  token_id_t& operator[](vertex_t vertex)
  {
    return fields_[vertex];
  }

  token_id_t operator[](vertex_t vertex) const
  {
    return fields_[vertex];
  }

  size_t size() const
  {
    return fields_.size();
  }

private:
  std::vector<std::unordered_map<edge_name_t,vertex_t> > neighbours_;
  std::vector<token_id_t> fields_;
};

#endif //RBGGAMEMANAGER_GRAPH_BOARD_H
