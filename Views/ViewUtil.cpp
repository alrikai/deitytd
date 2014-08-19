
#include "ViewUtil.hpp"

namespace OgreUtil
{

    namespace detail
    {
    void nuke_movables(Ogre::SceneNode* t_scenenode)
    {
        if(!t_scenenode)
            return;

        //remove the resources attached to the current scene node
        auto obj_it = t_scenenode->getAttachedObjectIterator();
        while (obj_it.hasMoreElements())
            t_scenenode->getCreator()->destroyMovableObject(obj_it.getNext());

        auto child_scenenode_it = t_scenenode->getChildIterator();
        while (child_scenenode_it.hasMoreElements())
        {
            auto child_scenenode = static_cast<Ogre::SceneNode*>(child_scenenode_it.getNext());
            nuke_scenenode(child_scenenode);    
        }
    }
    }


    void nuke_scenenode(Ogre::SceneNode* t_scenenode)
    {
       if(!t_scenenode) 
           return;

       detail::nuke_movables(t_scenenode);

       t_scenenode->removeAndDestroyAllChildren();
       t_scenenode->getCreator()->destroySceneNode(t_scenenode);
    }

} //namespace OgreUtil
