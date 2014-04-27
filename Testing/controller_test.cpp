#include "Controller.hpp"

#include <OGRE/Ogre.h>

#include <string>
#include <memory>
#include <iostream>
#include <chrono>

// Go through all sections & settings in the file
void load_resources(const std::string& resource_cfg_filename)
{
    Ogre::ConfigFile config;
    config.load(resource_cfg_filename);
    Ogre::ConfigFile::SectionIterator seci = config.getSectionIterator();
    std::string secName, typeName, archName;
    while (seci.hasMoreElements())
    {
        secName = seci.peekNextKey();
        Ogre::ConfigFile::SettingsMultiMap *settings = seci.getNext();
        Ogre::ConfigFile::SettingsMultiMap::iterator i;
        for (i = settings->begin(); i != settings->end(); ++i)
        {
            typeName = i->first;
            archName = i->second;
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(
                archName, typeName, secName);
        }
    }
}

//test harness for the Controller
int main()
{
    const std::string plugins_cfg_filename {"plugins.cfg"};
    Ogre::Root* root = new Ogre::Root(plugins_cfg_filename);   

    //load resources
    const std::string resource_cfg_filename {"resources.cfg"};
    load_resources(resource_cfg_filename);
    
    //configure the system
    if(!root->restoreConfig())
        if(!root->showConfigDialog()) 
           return -1;

    Ogre::RenderWindow* render_window = root->initialise(true, "Minimal OGRE");
    Controller controller (root, render_window);
    
    //now I guess we would have to do something while checking our keyboard and mouse listening capabilities   
    //... 
    Ogre::SceneManager* scene_mgmt = root->createSceneManager("OctreeSceneManager");
    Ogre::SceneNode* root_node = scene_mgmt->getRootSceneNode();
    Ogre::Camera* camera = scene_mgmt->createCamera("MinimalCamera");
    camera->setNearClipDistance(1.5);
    camera->setFarClipDistance(3000);
    //have a 4:3 aspect ratio
    camera->setAspectRatio(Ogre::Real(4.0f/3.0f));
    camera->setPosition(Ogre::Vector3(0,0,80)); 
    camera->lookAt(Ogre::Vector3(0,0,-300));
  
    Ogre::Viewport* view_port = render_window->addViewport(camera);
    view_port->setBackgroundColour(Ogre::ColourValue(0, 0, 0));

    Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();
     
    Ogre::Entity* ogre_head = scene_mgmt->createEntity("Head", "ogrehead.mesh");
    Ogre::SceneNode* head_node = root_node->createChildSceneNode("HeadNode");
    head_node->attachObject(ogre_head);
    Ogre::Light* light = scene_mgmt->createLight("MainLight");
    light->setPosition(20.0f, 80.0f, 50.0f);

//root->startRendering(); 

    double time_elapsed = 0;
    const double TOTAL_TIME = 10 * 1000;
    auto start_time = std::chrono::high_resolution_clock::now();
    do
    {
        root->renderOneFrame();
        Ogre::WindowEventUtilities::messagePump();
       
        auto end_time = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double, std::milli> time_duration (end_time - start_time);
        time_elapsed = time_duration.count();
    } while(time_elapsed < TOTAL_TIME);
}
