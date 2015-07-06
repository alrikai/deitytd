#ifndef TD_VIEW_UTIL_HPP
#define TD_VIEW_UTIL_HPP

#include "ModelUtils.hpp"
#include <OGRE/Ogre.h>

namespace OgreUtil
{
    //recursivly destroys all resources attached to a given scenenode
    void nuke_scenenode(Ogre::SceneNode* t_scenenode);
    
    void load_model(Ogre::SceneNode* t_scenenode, const CharacterModels::ModelIDs id, const std::string model_handleid);
}  //namespace OgreUtil

#endif
