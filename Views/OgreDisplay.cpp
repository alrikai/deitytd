
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
//    background->draw_background(view_port);

    double time_elapsed = 0;
    const double TOTAL_TIME = 10 * 1000;
    auto start_time = std::chrono::high_resolution_clock::now();
    do
    {
        root->renderOneFrame();
        Ogre::WindowEventUtilities::messagePump();
      
        //how best to do this? check the input queues for messages? In this thread, or in another one?
        handle_user_input();        

        auto end_time = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double, std::milli> time_duration (end_time - start_time);
        time_elapsed = time_duration.count();
    } while(time_elapsed < TOTAL_TIME);
 }


void OgreDisplay::draw_tower(const std::vector<std::vector<uint32_t>>& polygon_mesh, 
        const std::vector<std::vector<float>>& polygon_points, std::string tower_material_name, std::string&& tower_name)
{
    Ogre::ManualObject* tower_obj = scene_mgmt->createManualObject(tower_name + "_mesh"); 
    
    //have a default material to use?
    if(tower_material_name.empty())
        tower_material_name = "DefaultTower";

    tower_obj->begin(tower_material_name, Ogre::RenderOperation::OT_TRIANGLE_LIST);
    {
        for (auto& polygon_pt : polygon_points)
        {
            assert(polygon_pt.size() == 3); 
            tower_obj->position(polygon_pt.at(0), polygon_pt.at(1), polygon_pt.at(2));
            tower_obj->colour(Ogre::ColourValue(1.0f,1.0f,1.0f,1.0f)); 
        }
  
        for (auto& polygon_idx : polygon_mesh)
            tower_obj->triangle(polygon_idx.at(0), polygon_idx.at(1), polygon_idx.at(2));
    }
    tower_obj->end();
    tower_obj->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
  
    tower_obj->convertToMesh(tower_name); 
    
    
    Ogre::Entity* tower_entity = scene_mgmt->createEntity(tower_name);
    tower_entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
    auto child_node = root_node->createChildSceneNode();
    child_node->attachObject(tower_entity);
}


void OgreDisplay::setup_camera()
{
    camera = scene_mgmt->createCamera("MinimalCamera");
    camera->setNearClipDistance(5);
    camera->setFarClipDistance(5000);
    //have a 4:3 aspect ratio
    camera->setAspectRatio(Ogre::Real(4.0f/3.0f));
    camera->setPosition(Ogre::Vector3(0,0,500)); 
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


void OgreDisplay::register_input_controller(Controller* controller)
{
    const std::string id {"ThisShouldBeSomethingMeaningful"};
    controller->register_input_listener(id, input_events.get());
}


void OgreDisplay::handle_user_input()
{
    ControllerUtil::INPUT_TYPE ui_evt;

    auto camera_direction = Ogre::Vector3::ZERO;

    //take action based on the user input -- note: should we limit the user input rate?
    // or do this in another thread? 
    while(input_events->pop(ui_evt))
    {
        switch(ui_evt) 
        {
            case ControllerUtil::INPUT_TYPE::LArrow:
                camera_direction.x += -cam_move;
            break;
            case ControllerUtil::INPUT_TYPE::RArrow:
                camera_direction.x += cam_move;
            break;
            case ControllerUtil::INPUT_TYPE::UpArrow:
                camera_direction.z += -cam_move;
            break;
            case ControllerUtil::INPUT_TYPE::DArrow:
                camera_direction.z += cam_move;
            break;
            case ControllerUtil::INPUT_TYPE::A:
                std::cout << "Key A" << std::endl;    
            break;
            case ControllerUtil::INPUT_TYPE::S:
                std::cout << "Key S" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::W:
                std::cout << "Key W" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::D:
                std::cout << "Key D" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::Esc:
                std::cout << "Key Escape" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::LClick:
                std::cout << "Mouse Lclick" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::RClick:
                std::cout << "Mouse Rclick" << std::endl;
            break;
            default:
                std::cout << "Unknown Type" << std::endl;
        };


        ControllerUtil::print_input_type(ui_evt);
    }

    camera->move(camera_direction);

}

