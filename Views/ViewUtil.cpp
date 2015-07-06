
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


    void load_model(Ogre::SceneNode* t_scenenode, const CharacterModels::ModelIDs id, const std::string model_handleid)
    {
/*
      const auto model_id = CharacterModels::id_names.at(static_cast<int>(id));
      auto model_ent = t_scenenode->createEntity(model_handleid, model_id + ".mesh");
      model_ent->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
      t_scenenode->attachObject(model_ent);
*/
      /*
      auto model_idit = CharacterModels::id_names.find(id);
      if(model_idit != CharacterModels::id_names.end()) {
        auto model_ent = t_scenenode->createEntity(model_handleid, *(model_id) + ".mesh");
        model_ent->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
        t_scenenode->attachObject(model_ent);
      } else {
        //should we throw an exception? 
      }
      */
    }

} //namespace OgreUtil



