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

//returns whether the click intersected the GameMap, and if so, what the distance was  
std::tuple<bool, float, Ogre::MovableObject*> check_point(Ogre::SceneManager* scene_mgmt, Ogre::Viewport* view_port, const float x, const float y)
{
    std::cout << "Viewport Actual Dim: [" << view_port->getActualLeft() << ", " << view_port->getActualTop() 
              << " -- " << view_port->getActualHeight() << ", " << view_port->getActualWidth() << "]" << std::endl;
    std::cout << "Viewport Relative  Dim: [" << view_port->getLeft() << ", " << view_port->getTop() 
              << " -- " << view_port->getHeight() << ", " << view_port->getWidth() << "]" << std::endl;
      
    auto cam = view_port->getCamera();
    Ogre::Ray ray = cam->getCameraToViewportRay(x,y);
    std::cout << "Ray [" << x << ", " << y << "] origin: " << ray.getOrigin() << " @direction " << ray.getDirection() << std::endl;
    std::cout << "Ray @ near plane: " << ray.getPoint(cam->getNearClipDistance()) << "Ray @ far plane: " << ray.getPoint(cam->getFarClipDistance()) << std::endl;
    std::cout << "Ray (non-normalized) [" << x * view_port->getActualWidth() << ", " << y * view_port->getActualHeight() << "]" << std::endl;

    std::tuple<bool, float, Ogre::MovableObject*> click_intersection {false, 0, nullptr};
    auto r_query = scene_mgmt->createRayQuery(ray);
    r_query->setSortByDistance(true);
    auto& q_hits = r_query->execute();
    if(!q_hits.empty())
    {
        for (auto q_it = q_hits.begin(); q_it != q_hits.end(); ++q_it)
        {
            if(q_it->movable)
            {
                Ogre::MovableObject* obj = q_it->movable;
                std::cout << "moveable -- " << obj->getName() << ": " << obj->getMovableType() << " bounding: " << obj->getWorldBoundingBox() << std::endl;
                if(obj->getName() == GameBackground::map_name)
                {
                    std::get<0>(click_intersection) = true;
                    std::get<1>(click_intersection) = q_it->distance;
                    std::get<2>(click_intersection) = obj;

                    std::cout << "Ray Length: " << q_it->distance << std::endl;
                    return click_intersection;
                }
            }
            else if(q_it->worldFragment)
                std::cout << "world fragment" << std::endl;
            else
                std::cout << "???" << std::endl;
        }
    }
    else
        std::cout << "[" << x << ", " << y << "] Did not intersect with anything" << std::endl;
 
    return click_intersection;
}

}

const std::string OgreDisplay::resource_cfg_filename {"resources.cfg"};
const std::string OgreDisplay::plugins_cfg_filename {"plugins.cfg"};


//this is the main rendering loop
void OgreDisplay::start_display()
{
    background->draw_background();

    double time_elapsed = 0;
    const double TOTAL_TIME = 600 * 1000;
    auto start_time = std::chrono::high_resolution_clock::now();
    do
    {
        root->renderOneFrame();
        Ogre::WindowEventUtilities::messagePump();
      
        //how best to do this? check the input queues for messages? In this thread, or in another one?
        handle_user_input();        

        //////////////////////////////////////////////////////
        //for fun: try rotating a fractal
        //auto fractal_node = root_node->getChild("ViewTest");
        //fractal_node->yaw(Ogre::Radian(3.14159265/5000.0f));
        //////////////////////////////////////////////////////
 
        auto end_time = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double, std::milli> time_duration (end_time - start_time);
        time_elapsed = time_duration.count();
    } while(time_elapsed < TOTAL_TIME && !close_display.load());
 }


void OgreDisplay::setup_camera()
{
    camera = scene_mgmt->createCamera("MinimalCamera");
    camera->setNearClipDistance(5);
    camera->setFarClipDistance(6000);
    //have a 4:3 aspect ratio, looking back along the Z-axis (should we do Y-axis instead?) 
    camera->setAspectRatio(Ogre::Real(4.0f/3.0f));
    camera->setPosition(Ogre::Vector3(0,0,300)); 
    camera->lookAt(Ogre::Vector3(0,0,0));
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


void OgreDisplay::generate_tower_event(const float x_coord, const float y_coord, const float click_distance, const Ogre::AxisAlignedBox& map_box)
{
    Ogre::Ray ray = camera->getCameraToViewportRay(x_coord/view_port->getActualWidth(), y_coord/view_port->getActualHeight());
    auto world_click = ray.getPoint(click_distance);

    std::cout << "World Click: " << world_click << " vs " << map_box << std::endl;
    //convert the click to map coordinates
    Ogre::Vector3 map_coord_mapping = (map_box.getMaximum() - map_box.getMinimum());

    float n_map_col = 0;
    float n_map_row = 0;
    if(world_click.x < 0)
        n_map_col = 0.5f - std::abs(world_click.x / map_coord_mapping.x);
    else
        n_map_col = 0.5f + std::abs(world_click.x / map_coord_mapping.x);

    if(world_click.y < 0)
        n_map_row = 0.5f - std::abs(world_click.y / map_coord_mapping.y);
    else
        n_map_row = 0.5f + std::abs(world_click.y / map_coord_mapping.y);
  
    const float norm_mapcoords_col = n_map_col; 
    const float norm_mapcoords_row = n_map_row;
    std::cout << world_click << " --> [" << norm_mapcoords_col << ", " << norm_mapcoords_row << "]" << std::endl;

/*
 * instead, what we should do is notify the gameloop that a tower has been placed by the user -- 
 * (or rather, we could have a queue of tower generation events that the game loop will read on
 * each iteration), and the game loop will notify the backend of a tower generation action (with
 * the pertinant information). The backend will then act on the user's tower generation action,
 * and give the gameloop back one or more tower models, which the game loop will give to the
 * frontend to render. 
 */
    const int tier = 1;
    tower_build_evtqueue->push(UserTowerEvents::build_tower_event(tier, norm_mapcoords_row, norm_mapcoords_col));
    return;
}

void OgreDisplay::place_tower(TowerModel* selected_tower, const std::string& tower_name, Ogre::Vector3 map_coord_offsets, Ogre::Vector3 world_coord_offsets)
{
    /*
    bool is_valid;
    std::string tower_name;
    TowerModel* selected_tower;
    Ogre::Vector3 map_coord_offsets   {0, 0, 0};
    Ogre::Vector3 world_coord_offsets {0, 0, 0};
    //return the offset in map coordinates
    std::tie(is_valid, tower_name, selected_tower) = td_model->make_tower(norm_mapcoords_col, norm_mapcoords_row, world_coord_offsets, map_coord_offsets);
    if(!is_valid)
        return;
    */

    std::cout << "TileCenter Offset: [" << map_coord_offsets.x << ", " << map_coord_offsets.y << "]" << std::endl;
    //NOTE: we want to have the tower ABOVE the map -- thus, its z coordinate has to be non-zero 
    const Ogre::Vector3 target_location { map_box.getHalfSize().x * (2 * (map_coord_offsets.x - 0.5f)), 
                                          map_box.getHalfSize().y * (2 * (map_coord_offsets.y - 0.5f)),
                                          world_coord_offsets.z};    

    Ogre::ManualObject* tower_obj = scene_mgmt->createManualObject(tower_name); 
    std::string tower_material_name {selected_tower->tower_material_name_};
    //have a default material to use?
    if(tower_material_name.empty())
        tower_material_name = "BaseWhiteNoLighting";

    tower_obj->begin(tower_material_name, Ogre::RenderOperation::OT_TRIANGLE_LIST);
    {
        for (const auto& polygon_pt : selected_tower->polygon_points_)
        {
            assert(polygon_pt.size() == 3); 
            //NOTE: the position is set as (x,y,z), but the ordering of the data I generate is (y,x,z).
            //should probably change all my ordering to be (x,y,z) to interoperate with Ogre better
            tower_obj->position(polygon_pt.at(1) - world_coord_offsets.x, 
                                polygon_pt.at(0) - world_coord_offsets.y,
                                polygon_pt.at(2) - world_coord_offsets.z);
            tower_obj->colour(Ogre::ColourValue(1.0f, 1.0f, 1.0f, 1.0f));
        }
  
        for (const auto& polygon_idx : selected_tower->polygon_mesh_)
            tower_obj->triangle(polygon_idx.at(0), polygon_idx.at(1), polygon_idx.at(2));
    }
    tower_obj->end();


    tower_obj->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
    auto tower_mesh = tower_obj->convertToMesh(tower_name); 
    
    Ogre::Entity* tower_entity = scene_mgmt->createEntity(tower_mesh);
    tower_entity->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
    auto child_node = root_node->createChildSceneNode(tower_name);
    child_node->attachObject(tower_entity);   
     
    //TODO: want the scale to be based on a few factors, such as the resolution, map size, and fractal dimensions
    const float tower_scale = 1.0f/4.0f;

    //NOTE: position is (x, y, z)
    child_node->setPosition(target_location.x, target_location.y, tower_scale * target_location.z);
    child_node->scale(tower_scale, tower_scale, tower_scale);
    //child_node->showBoundingBox(true);

    /*
    std::vector<std::string> particle_types
    {
    "Examples/Aureola",
    "Examples/GreenyNimbus",
    "Examples/Swarm",
    "Examples/PurpleFountain"
    };
    */
    std::vector<std::string> particle_types
    {
    "Tower/GreenGlow",
    "Tower/Sparks"
    };

    std::random_device rdev{};
    std::default_random_engine eng{rdev()};
    std::uniform_int_distribution<> dis(0, particle_types.size()-1);
    
    std::string particle_type {particle_types.at(dis(eng))}; 
    

    Ogre::ParticleSystem* particle_node = scene_mgmt->createParticleSystem(tower_name + "_particle", particle_type);

    //for testing -- see if we can properly modify the particlesystems 
    if(particle_type == "Tower/GreenGlow")
    {
        particle_node->getEmitter(0)->setColour(Ogre::ColourValue(1.0f, 0.0f, 0.0f, 1.0f));
    }
    child_node->attachObject(particle_node);


}


void OgreDisplay::handle_user_input()
{
    ControllerUtil::InputEvent ui_evt;
    auto camera_direction = Ogre::Vector3::ZERO;

    //NOTE: need something a bit less shitty of a solution than this
    bool valid_click = false;
    float click_distance = 0;
    Ogre::MovableObject* obj = nullptr;
    const float height = view_port->getActualHeight(); 
    const float width = view_port->getActualWidth();         

    //for mouse dragging, we can move as roll, pitch, yaw based on the mouse movement
    float cam_yaw = 0;
    float cam_pitch = 0;

    //take action based on the user input -- note: should we limit the user input rate?
    // or do this in another thread? 
    while(input_events->pop(ui_evt))
    {
        switch(ui_evt.event_type) 
        {
            case ControllerUtil::INPUT_TYPE::LArrow:
                camera_direction.x += -cam_move;
            break;
            case ControllerUtil::INPUT_TYPE::RArrow:
                camera_direction.x += cam_move;
            break;
            case ControllerUtil::INPUT_TYPE::UpArrow:
                camera_direction.y += -cam_move;
            break;
            case ControllerUtil::INPUT_TYPE::DArrow:
                camera_direction.y += cam_move;
            break;
            case ControllerUtil::INPUT_TYPE::PDown:
                camera_direction.z += -cam_move;
            break;
            case ControllerUtil::INPUT_TYPE::PUp:
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
            case ControllerUtil::INPUT_TYPE::ZERO:
                std::cout << "Key 0" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::ONE:
                std::cout << "Key 1" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::TWO:    
                std::cout << "Key 2" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::THREE:    
                std::cout << "Key 3" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::FOUR:    
                std::cout << "Key 4" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::FIVE:    
                std::cout << "Key 5" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::SIX:    
                std::cout << "Key 6" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::SEVEN: 
                std::cout << "Key 7" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::EIGHT:    
                std::cout << "Key 8" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::NINE:
                std::cout << "Key 9" << std::endl;
            break;
            case ControllerUtil::INPUT_TYPE::Esc:
                close_display = true;
            break;
            case ControllerUtil::INPUT_TYPE::LClick:
                std::cout << "Mouse Lclick @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
                check_point(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);
            break;
            case ControllerUtil::INPUT_TYPE::RClick:
                std::cout << "Mouse Rclick @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
                std::tie(valid_click, click_distance, obj) = check_point(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);
                if(valid_click)
                {
                    std::cout << "Total Dims: " << obj->getWorldBoundingBox() << std::endl;
                    generate_tower(ui_evt.x_pos, ui_evt.y_pos, click_distance, obj->getWorldBoundingBox());
                }
            break;
            case ControllerUtil::INPUT_TYPE::MDrag:
                //provides the difference in current mouse pos. from the previous mouse pos.
                std::cout << "Mouse Drag @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
                 cam_yaw = ui_evt.x_pos;
                 cam_pitch = ui_evt.y_pos;
            break;
            default:
                std::cout << "Unknown Type" << std::endl;
        };

        ControllerUtil::print_input_type(ui_evt.event_type);
    }

    camera->move(camera_direction);
    camera->yaw(Ogre::Degree(cam_yaw)*-0.2f);
    camera->pitch(Ogre::Degree(cam_pitch)*-0.2f);
}


