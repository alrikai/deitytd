/* ModelUtils.hpp -- part of the DietyTD Common implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_INCLUDE_COMMON_MODEL_UTILS_HPP
#define TD_INCLUDE_COMMON_MODEL_UTILS_HPP

#include <string>
#include <vector>
//#include <map>

//the game states -- (at the moment), it's just in-round (mobs running, towers attacking, etc) and
//idle (between rounds, so towers building, upgrading, etc).
enum class GAME_STATE { ACTIVE, IDLE, PAUSED };

struct CharacterModels
{
  enum class ModelIDs
  {
    ogre_S = 0
  };

  //might not want / need this here (need the model string on the front-end side)
  //static const std::map <ModelIDs, std::string> id_names;
  static const std::vector<std::string> id_names;
};

#endif
