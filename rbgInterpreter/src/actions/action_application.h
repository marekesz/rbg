//
// Created by ??? on 19/02/18.
//

#ifndef RBGGAMEMANAGER_ACTION_APPLICATION_H
#define RBGGAMEMANAGER_ACTION_APPLICATION_H

#include "../game_components/name_resolver.h"
#include "../game_components/graph_board.h"

class Action;

// Object representing action application at some position.
class ActionApplication {
public:
  ActionApplication(vertex_t applied_pos, const Action *action)
      : applied_pos_(applied_pos),
        applied_action_(action) {}

  vertex_t pos() const { return  applied_pos_; }

  const Action *action() const { return applied_action_; }

private:
  vertex_t applied_pos_;
  const Action *applied_action_;
};

// Object representing the result of Action Apply function.
// Can be used to revert the Action by calling Action Revert.
class ActionResult {
public:
  ActionResult(bool success)
      : success_(success), information_(0) {}

  ActionResult(bool success, token_id_t information)
      : success_(success), information_(information) {}

  ActionResult(bool success, int information)
      : success_(success), revert_value_(information) {}

  ActionResult(bool success, vertex_t information)
      : success_(success), revert_pos_(information)
  {}

  // Returns true when the action was successful.
  operator bool() const { return success_; }

  token_id_t revert_piece() const { return revert_piece_; }

  token_id_t revert_player() const { return revert_player_; }

  int revert_value() const { return revert_value_; }

  vertex_t revert_pos() const { return revert_pos_; }

private:
  bool success_;
  // ActionResults are objects that are stored on the stack
  // while making a search, that is why they must be small
  // objects. This is the reason of this union.
  union {
    token_id_t information_;
    token_id_t revert_piece_;
    token_id_t revert_player_;
    vertex_t revert_pos_;
    int revert_value_;
  };
};

#endif //RBGGAMEMANAGER_ACTION_APPLICATION_H
