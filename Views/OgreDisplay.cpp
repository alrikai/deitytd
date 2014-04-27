
#include "OgreDisplay.hpp"

namespace
{
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
}

const std::string OgreDisplay::resource_cfg_filename {"resources.cfg"};
const std::string OgreDisplay::plugins_cfg_filename {"plugins.cfg"};


void OgreDisplay::start_display()
{
    background->draw_background(view_port);

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

    //root->startRendering();
    /*
    auto start = std::chrono::system_clock::now();

    while(true)
    {
        render_window->update(false);
        render_window->swapBuffers();

        root->renderOneFrame();
        Ogre::WindowEventUtilities::messagePump();

        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>
            (std::chrono::system_clock::now() - start);
        if(duration > std::chrono::milliseconds(10000))
            break;
    }
    */
}

void OgreDisplay::setup_camera()
{
    camera = scene_mgmt->createCamera("MinimalCamera");
    camera->setNearClipDistance(1.5);
    camera->setFarClipDistance(3000);
    //have a 4:3 aspect ratio
    camera->setAspectRatio(Ogre::Real(4.0f/3.0f));
    camera->setPosition(Ogre::Vector3(0,0,80)); 
    camera->lookAt(Ogre::Vector3(0,0,-300));
}

bool OgreDisplay::ogre_setup()
{
    //load resources
    load_resources(resource_cfg_filename);
    
    //configure the system
    if(!root->restoreConfig())
        if(!root->showConfigDialog())
            return false;

    render_window = root->initialise(true, "Minimal OGRE");
    scene_mgmt = root->createSceneManager("OctreeSceneManager");
    root_node = scene_mgmt->getRootSceneNode();
	return true;
}



