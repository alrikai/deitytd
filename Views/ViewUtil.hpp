#ifndef TD_VIEW_UTIL_HPP
#define TD_VIEW_UTIL_HPP

#include <OGRE/Ogre.h>

namespace OgreUtil
{
    //recursivly destroys all resources attached to a given scenenode
    void nuke_scenenode(Ogre::SceneNode* t_scenenode);
    
}  //namespace OgreUtil

#endif
