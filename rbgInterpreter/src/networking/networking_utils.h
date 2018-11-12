//
// Created by ??? on 01/02/18.
//

#ifndef RBGGAMEMANAGER_UTILS_H
#define RBGGAMEMANAGER_UTILS_H

#include <string>
#include <sstream>
#include "../game_components/search_context.h"
#include "../game_components/name_resolver.h"

class Message;

std::string MoveDataString(const Move &move);

Move DecodeMove(const Message &message);

#endif //RBGGAMEMANAGER_UTILS_H
