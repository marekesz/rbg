//
// Created by ??? on 06/02/18.
//

#ifndef RBGGAMEMANAGER_UTILITIES_H
#define RBGGAMEMANAGER_UTILITIES_H

#include "../actions/action.h"
#include "../game_components/game_description.h"

std::vector<const Action *>
get_actions_in_block(const GameMovesDescription &gd, size_t block_id) {
  std::vector<const Action *> result;
  fsm::state_id_t current = gd.CorrespondingState(block_id);
  while (gd.nfa()[current].transitions().size() == 1 &&
         gd.nfa()[current].transitions().front().letter()->index() ==
         block_id) {
    result.push_back(gd.nfa()[current].transitions().front().letter());
    current = gd.nfa()[current].transitions().front().target();
  }
  return result;
}

#endif //RBGGAMEMANAGER_UTILITIES_H
