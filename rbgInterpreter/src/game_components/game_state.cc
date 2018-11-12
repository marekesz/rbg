//
// Created by ??? on 12.01.18.
//

#include <iomanip>
#include <cmath>
#include <sstream>
#include "game_state.h"
#include <algorithm>

extern unsigned int kTextWidth;

ActionResult
GameState::ApplyActionApplication(const ActionApplication &application) {
  vertex_t old_pos = current_pos_;
  current_pos_ = application.pos();
  auto res = application.action()->Apply(this);
  current_pos_ = old_pos;
  return res;
}

void
GameState::RevertActionApplication(const ActionApplication &application,
                                   const ActionResult &application_result) {
  vertex_t old_pos = current_pos_;
  current_pos_ = application.pos();
  application.action()->Revert(this, application_result);
  current_pos_ = old_pos;
}

std::ostream &outNormalBoard(std::ostream &s,const GameState &state)
{
  auto edge_size = static_cast<size_t>(std::sqrt(state.board().size()));
  if(edge_size*edge_size != state.board().size())
  {
    edge_size = static_cast<int>(state.board().size());
  }

  int text_width = 2;
  for (vertex_t v = 0; v < static_cast<ssize_t >(state.board().size()); v++)
  {
    std::string name = state.description().resolver().Name(
        state.board()[v]);
    if (name == "empty" || name == "e") {
      s << "[" << std::setw(text_width) << " " << "] ";
    } else {
      s << "[" << std::setw(text_width) << name.substr(0, (unsigned long) text_width)
        << "] ";
    }
    if((v + 1) % edge_size == 0)
      s << '\n';
  }
  return s;
}

std::ostream &outRectangleBoard(std::ostream &s,const GameState &state)
{
  size_t width = 0;
  size_t height = 0;
  for (vertex_t v = 0; v < static_cast<ssize_t >(state.board().size()); v++)
  {
    std::string vertex_name = state.description().vertex_resolver().Name(v);
    std::stringstream stream(vertex_name);
    char placeholders[3] = {'\0','\0','\0'};
    stream >> placeholders[0] >> placeholders[1];
    if(std::string(placeholders) != "rx")
    {
      return outNormalBoard(s,state);
    }
    size_t x,y;
    char placeholder;
    stream >> x >> placeholder >> y;
    if(placeholder != 'y')
    {
      return outNormalBoard(s,state);
    }
    width = std::max(x+1,width);
    height = std::max(y+1,height);
  }
  std::vector<token_id_t> board(width * height);
  for (vertex_t v = 0; v < static_cast<ssize_t >(state.board().size()); v++)
  {
    std::string vertex_name = state.description().vertex_resolver().Name(v);
    std::stringstream stream(vertex_name);
    char placeholder;
    stream >> placeholder >> placeholder;
    size_t x,y;
    stream >> x >> placeholder >> y;
    board[x + y * width] = state.board()[v];
  }
  for(size_t y = 0; y < height; y++)
  {
    for(size_t x = 0; x < width; x++)
    {
      std::string name = state.description().resolver().Name(board[x + y * width]);
      if (name == "empty" || name == "e") {
        s << "[" << std::setw(kTextWidth) << " " << "] ";
      } else {
        s << "[" << std::setw(kTextWidth) << name.substr(0, (unsigned long) kTextWidth)
          << "] ";
      }
    }
    s << '\n';
  }
  return s;
}

std::ostream &outHexagonBoard(std::ostream &s,const GameState &state)
{
  size_t width = 0;
  size_t height = 0;
  for (vertex_t v = 0; v < static_cast<ssize_t >(state.board().size()); v++)
  {
    std::string vertex_name = state.description().vertex_resolver().Name(v);
    std::stringstream stream(vertex_name);
    char placeholders[3] = {'\0','\0','\0'};
    stream >> placeholders[0] >> placeholders[1];
    if(std::string(placeholders) != "hx")
    {
      return outNormalBoard(s,state);
    }
    size_t x,y;
    char placeholder;
    stream >> x >> placeholder >> y;
    if(placeholder != 'y')
    {
      return outNormalBoard(s,state);
    }
    width = std::max(x+1,width);
    height = std::max(y+1,height);
  }
  std::vector<token_id_t> board(width * height, state.description().resolver().NamesCount());
  std::vector<size_t> padding(height,width);
  for (vertex_t v = 0; v < static_cast<ssize_t >(state.board().size()); v++)
  {
    std::string vertex_name = state.description().vertex_resolver().Name(v);
    std::stringstream stream(vertex_name);
    char placeholder;
    stream >> placeholder >> placeholder;
    size_t x,y;
    stream >> x >> placeholder >> y;
    board[x + y * width] = state.board()[v];
  }
  for(size_t y = 0; y < height; y++)
  {
    for(size_t x = 0; x < std::max(y,height-y-1) * (kTextWidth + 3) / 2; x++)
      std::cout << " ";
    for(size_t x = 0; x < width; x++)
    {
      if(board[x + y * width] != state.description().resolver().NamesCount()) {
        std::string name = state.description().resolver().Name(board[x + y * width]);
        if (name == "empty" || name == "e") {
          s << "[" << std::setw(kTextWidth) << " " << "] ";
        } else {
          s << "[" << std::setw(kTextWidth) << name.substr(0, (unsigned long) kTextWidth)
            << "] ";
        }
      }
      else
      {
          s << std::setw(kTextWidth+3) << " ";
      }
    }
    s << '\n';
  }
  return s;
}


std::ostream &operator<<(std::ostream &s, const GameState &state) {
  s << "Player: " << state.description().resolver().Name(state.player())
    << "\n";
  s << "Variables: ";
  for (token_id_t i = 0; i < state.description().VariablesCount(); i++) {
    s << state.description().resolver().Name(i) << ":" << state.Value(i);
    if (i != state.description().VariablesCount() - 1)
      s << ", ";
  }
  s << "\n";
  s << "Position: " << state.pos() << "[NFA state: "
    << state.nfa_state() << "]\n";
  if(state.description().vertex_resolver().Name(0).substr(0,2) == "rx")
  {
    outRectangleBoard(s,state);
  }
  else if(state.description().vertex_resolver().Name(0).substr(0,2) == "hx")
  {
    outHexagonBoard(s,state);
  }
  else
  {
    outNormalBoard(s,state);
  }
  return s;
}
