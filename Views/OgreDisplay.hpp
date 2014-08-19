#ifndef TD_OGRE_DISPLAY_HPP
#define TD_OGRE_DISPLAY_HPP

#include "ControllerUtil.hpp"
#include "Controller.hpp"

#include "GameBackground.hpp"
#include "ViewEventTypes.hpp"
#include "ViewUtil.hpp"

#include "TowerDefense.hpp"
#include "util/Types.hpp"

#include <opencv2/opencv.hpp>
#include <OGRE/Ogre.h>

#include <iostream>
#include <memory>
#include <chrono>
#include <string>
#include <atomic>

template <class BackendType>
class OgreDisplay
{

public:
    using TowerEventQueueType = typename UserTowerEvents::EventQueueType<UserTowerEvents::tower_event<BackendType>>::QType; 
    using RenderEventQueueType = RenderEvents::MakeTowerQType; 
    using RenderAttackEventQueueType = RenderEvents::MakeAttackQType;
    using RenderAttackMoveEventQueueType = RenderEvents::MakeAttackMoveQType;

    OgreDisplay()
        : root (new Ogre::Root(plugins_cfg_filename)), cam_rotate(0.10f), cam_move(10.0f),
        background(nullptr), close_display(false)
    {
        ogre_setup();
        setup_camera(); 

		//not really sure if these should get their own method, or be rolled into an existing one. So for now, just have them here...
		view_port = render_window->addViewport(camera);
        view_port->setBackgroundColour(Ogre::ColourValue(0, 0, 0));
        Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();

        main_light = scene_mgmt->createLight("MainLight");
        main_light->setType(Ogre::Light::LightTypes::LT_DIRECTIONAL);
        main_light->setDiffuseColour(Ogre::ColourValue(.25, .25, 0));
        main_light->setSpecularColour(Ogre::ColourValue(.25, .25, 0));
        main_light->setDirection(Ogre::Vector3(0,-1,1));

        spot_light = scene_mgmt->createLight("OtherLight");
        spot_light->setPosition(20.0f, 80.0f, 50.0f); 

        Ogre::Light* spotLight = scene_mgmt->createLight("spotLight");
        spotLight->setType(Ogre::Light::LT_SPOTLIGHT);
        spotLight->setDiffuseColour(0, 0, 1.0);
        spotLight->setSpecularColour(0, 0, 1.0);
        spotLight->setDirection(-1, -1, 0);
        spotLight->setPosition(Ogre::Vector3(0, 0, 300));
        spotLight->setSpotlightRange(Ogre::Degree(35), Ogre::Degree(50));

        background.reset(new GameBackground(scene_mgmt, view_port));
        input_events = std::unique_ptr<ControllerUtil::ControllerBufferType>(new ControllerUtil::ControllerBufferType());


        render_event_queue = std::unique_ptr<RenderEventQueueType>(new RenderEventQueueType());
        render_attackevent_queue = std::unique_ptr<RenderAttackEventQueueType>(new RenderAttackEventQueueType());
        render_attackmove_event_queue = std::unique_ptr<RenderAttackMoveEventQueueType>(new RenderAttackMoveEventQueueType());
    }

	void start_display();

    void place_tower(TowerModel* selected_tower, const std::string& tower_name, const Ogre::AxisAlignedBox& map_box,
                     Ogre::Vector3 map_coord_offsets, Ogre::Vector3 world_coord_offsets);
    void register_input_controller(Controller* controller)
    {
        const std::string id {"ThisShouldBeSomethingMeaningful"};
        controller->register_input_listener(id, input_events.get());
    }

    void register_tower_eventqueue(TowerEventQueueType* tevt_queue)
    {
        td_event_queue = tevt_queue;
    }

    /////////////////////////////////////////////////////////////////////
    void add_render_event(std::unique_ptr<RenderEvents::create_tower> evt)
    {
        std::cout << "Adding Render Event -- @name " << evt->t_name << std::endl;
        render_event_queue->push(std::move(evt)); 
    }
    void add_atk_event(std::unique_ptr<RenderEvents::create_attack> evt)
    {
        render_attackevent_queue->push(std::move(evt));
    }
    void add_atkmove_event(std::unique_ptr<RenderEvents::move_attack> evt)
    {
        std::cout << "Adding Attack Move Event @ View" << std::endl;
        render_attackmove_event_queue->push(std::move(evt));
    }
    /////////////////////////////////////////////////////////////////////


    Ogre::Root* get_root() const
    { return root.get(); }
    Ogre::RenderWindow* get_render_window() const 
    { return render_window; }
	Ogre::SceneManager* get_scene_mgmt() const
    { return scene_mgmt; }

    void draw_maptiles(const int num_col_tiles, const int num_row_tiles)
    { background->draw_tiles(num_col_tiles, num_row_tiles); }

private:        
    bool ogre_setup();
    void setup_camera(); 
    void setup_background();
    void handle_user_input();

    void generate_tower(const float x_coord, const float y_coord, const float click_distance, const Ogre::AxisAlignedBox& mapobj);

    const static std::string resource_cfg_filename; 
    const static std::string plugins_cfg_filename;

    std::unique_ptr<Ogre::Root> root;

    Ogre::RenderWindow* render_window;
	Ogre::SceneManager* scene_mgmt;
    Ogre::SceneNode* root_node;

    //the coefficients for camera movement (there will be more as we add more functionality)
    const float cam_rotate;
    const float cam_move;

	Ogre::Camera* camera;
    Ogre::Viewport* view_port;
    Ogre::Light* main_light;
    Ogre::Light* spot_light;

    std::unique_ptr<GameBackground> background;
    std::unique_ptr<ControllerUtil::ControllerBufferType> input_events;

    //the tower event queue -- owned by the TowerDefense class
    TowerEventQueueType* td_event_queue;


    std::unique_ptr<RenderEventQueueType> render_event_queue;
    std::unique_ptr<RenderAttackEventQueueType> render_attackevent_queue;
    std::unique_ptr<RenderAttackMoveEventQueueType> render_attackmove_event_queue;

    //the plan is to eventually have multiple threads running, so making this
    //atomic ahead of time (although this might change in the future...)
    std::atomic<bool> close_display;

    //holds the state of the current selection of the user (or nullptr if none selected)
    //this will presumably either be towers or mobs -- wouldn't make sense to be able to click attacks
    Ogre::MovableObject* current_selection;

    /*
    //we want to render things when the backend sends the updated locations -- we could go about this a couple of ways,
    //but the one I'm leaning towards now is having the rendering thread waiting on the updated locations
    std::atomic<bool> render_ready;
    std::list<TowerModel> render_towers;
    std::condition_variable render_cv;
    std::mutex render_mutx;
    */
};






template <class BackendType>
const std::string OgreDisplay<BackendType>::resource_cfg_filename {"resources.cfg"};

template <class BackendType>
const std::string OgreDisplay<BackendType>::plugins_cfg_filename {"plugins.cfg"};


//this is the main rendering loop
template <class BackendType>
void OgreDisplay<BackendType>::start_display()
{
    background->draw_background();

    //10 FPS is the bare minimum
    constexpr int max_ms_perframe = 100;

    double time_elapsed = 0;
    const double TOTAL_TIME = 600 * 1000;
    auto start_time = std::chrono::high_resolution_clock::now();
    do
    {
/*
        //render when triggered, or when the maximum allowable time has elapsed
        std::unique_lock<std::mutex> render_lk(render_mutx);
        render_cv.wait_for(render_lk, max_ms_perframe, std::chrono::milliseconds(), 
                []{return render_ready.load()});
*/
        root->renderOneFrame();
        Ogre::WindowEventUtilities::messagePump();
      
        //how best to do this? check the input queues for messages? In this thread, or in another one?
        handle_user_input();        


        /////////////////////////////////////////////////////////////////////////////////////////////
        //TODO: handling input rendering events should be factored out into its own function or class
        /////////////////////////////////////////////////////////////////////////////////////////////
        while(!render_event_queue->empty())
        {
            bool got_evt = false;
            auto render_evt = render_event_queue->pop(got_evt);
            if(got_evt && render_evt)
            {
                Ogre::Vector3 t_map_offsets {render_evt->t_map_offsets[0], render_evt->t_map_offsets[1], render_evt->t_map_offsets[2]};
                Ogre::Vector3 t_world_offsets {render_evt->t_world_offsets[0], render_evt->t_world_offsets[1], render_evt->t_world_offsets[2]};                 

                //NOTE: this is the last piece of the puzzle. Should be the GameMap (GameBGMap), but I formally would get it from a scene query.
                Ogre::AxisAlignedBox map_box = background->get_map_aab();
                place_tower(render_evt->t_model.get(), render_evt->t_name, map_box, t_map_offsets, t_world_offsets);
            }
        }
        
        while(!render_attackevent_queue->empty())
        {
            bool got_evt = false;
            auto render_evt = render_attackevent_queue->pop(got_evt);
            if(got_evt && render_evt)
            {
                //NOTE: these coordinates are in terms of TILES. Need to convert to world coords, or at least to have the
                //attack originate from the tower that spawned it. How best to do this?
                auto origin_id = render_evt->origin_id;

               
                /*
                auto t_origin = scene_mgmt->getEntity(origin_id)->getBoundingBox();
                std::cout << "T Origin: " << t_origin << std::endl;
                Ogre::Vector3 origin = t_origin.getCenter();
                */
                Ogre::Vector3 origin = scene_mgmt->getEntity(origin_id)->getParentSceneNode()->getPosition();

                auto dest = render_evt->target;
                std::cout << " Made attack @ [" << origin[0] << ", " << origin[1] << ", " << origin[2] << "] --> [" 
                     << dest[0] << ", " << dest[1] << ", " << dest[2] << "]" << std::endl;

                Ogre::Entity* tower_atk = scene_mgmt->createEntity(render_evt->name, Ogre::SceneManager::PT_SPHERE);
                tower_atk->setMaterialName("Examples/Chrome");
                tower_atk->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
                auto child_node = root_node->createChildSceneNode(render_evt->name);

                //scale the attack to a more sensible size...
                Ogre::Vector3 scale_vec(1/25.0f,1/25.0f, 1/25.0f);
                child_node->setScale(scale_vec);
                child_node->setPosition(origin.x, origin.y, origin.z);
                child_node->attachObject(tower_atk);   
            }
        }

        while(!render_attackmove_event_queue->empty())
        {
            bool got_evt = false;
            auto render_evt = render_attackmove_event_queue->pop(got_evt);
            if(got_evt && render_evt)
            {
                //TODO
                std::cout << "Move event @ " << render_evt->name << std::endl;

                auto attack_id = render_evt->name;
                Ogre::Vector3 movement {render_evt->delta[0], render_evt->delta[1], 0};
                scene_mgmt->getEntity(attack_id)->getParentSceneNode()->translate(movement);
            }
        }
        
        /////////////////////////////////////////////////////////////////////////////////////////////

         
        auto end_time = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double, std::milli> time_duration (end_time - start_time);
        time_elapsed = time_duration.count();
    } while(time_elapsed < TOTAL_TIME && !close_display.load());
 }


template <class BackendType>
void OgreDisplay<BackendType>::setup_camera()
{
    camera = scene_mgmt->createCamera("MinimalCamera");
    camera->setNearClipDistance(5);
    camera->setFarClipDistance(6000);
    //have a 4:3 aspect ratio, looking back along the Z-axis (should we do Y-axis instead?) 
    camera->setAspectRatio(Ogre::Real(4.0f/3.0f));
    camera->setPosition(Ogre::Vector3(0,0,300)); 
    camera->lookAt(Ogre::Vector3(0,0,0));
}


template <class BackendType>
bool OgreDisplay<BackendType>::ogre_setup()
{
    //load resources
    view_detail::load_resources(resource_cfg_filename);
    
    //configure the system
    if(!root->restoreConfig())
        if(!root->showConfigDialog())
            return false;

    render_window = root->initialise(true, "Minimal OGRE");
    scene_mgmt = root->createSceneManager("OctreeSceneManager");
    root_node = scene_mgmt->getRootSceneNode();
    return true;
}


template <class BackendType>
void OgreDisplay<BackendType>::generate_tower(const float x_coord, const float y_coord, const float click_distance, const Ogre::AxisAlignedBox& map_box)
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
    using tower_evt_t = UserTowerEvents::build_tower_event<BackendType>;
    std::unique_ptr<UserTowerEvents::tower_event<BackendType>> td_evt = 
        std::unique_ptr<tower_evt_t> (new tower_evt_t(tier, norm_mapcoords_row, norm_mapcoords_col));
    td_event_queue->push(std::move(td_evt));
    return;
}

template <class BackendType>
void OgreDisplay<BackendType>::place_tower(TowerModel* selected_tower, const std::string& tower_name, const Ogre::AxisAlignedBox& map_box, 
                                           Ogre::Vector3 map_coord_offsets, Ogre::Vector3 world_coord_offsets)
{
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
    
    Ogre::Entity* tower_entity = scene_mgmt->createEntity(tower_name, tower_mesh);
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
        particle_node->getEmitter(0)->setColour(Ogre::ColourValue(0.0f, 0.0f, 1.0f, 1.0f));
    }
    child_node->attachObject(particle_node);
}


template <class BackendType>
void OgreDisplay<BackendType>::handle_user_input()
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
            {
                camera_direction.x += -cam_move;
                break;
            }
            case ControllerUtil::INPUT_TYPE::RArrow:
            {
                camera_direction.x += cam_move;
                break;
            }
            case ControllerUtil::INPUT_TYPE::UpArrow:
            { 
                camera_direction.y += -cam_move;
                break;
            }
            case ControllerUtil::INPUT_TYPE::DArrow:
            {
                camera_direction.y += cam_move;
                break;
            }
            case ControllerUtil::INPUT_TYPE::PDown:
            { 
                camera_direction.z += -cam_move;
                break;
            }
            case ControllerUtil::INPUT_TYPE::PUp:
            {
                camera_direction.z += cam_move;
                break;
            }
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
            {
                //have D mean deletion of the current object. This will likely change once we add a real GUI...
                if(current_selection)
                {
                    std::cout << "Deleting object " << current_selection->getName() << std::endl;
                    Ogre::SceneNode* t_scenenode = current_selection->getParentSceneNode();
                    OgreUtil::nuke_scenenode(t_scenenode);


                    //TODO: enqueue a tower delete event for the backend
                }
                break;
            }
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
            {
                std::cout << "Mouse Lclick @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
                
                //NOTE: current_selection is set to nullptr if nothing was selected. Might want to disallow selecting the game map and certain other objects
                current_selection = view_detail::user_select(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);
                if(current_selection)
                    std::cout << "Selected " << current_selection->getName() << " @ " << current_selection->getParentSceneNode()->getPosition() << std::endl;
                break;
            }
            case ControllerUtil::INPUT_TYPE::RClick:
            {
                std::cout << "Mouse Rclick @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
                std::tie(valid_click, click_distance, obj) = view_detail::check_point(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);
                if(valid_click)
                {
                    std::cout << "Total Dims: " << obj->getWorldBoundingBox() << std::endl;
                    generate_tower(ui_evt.x_pos, ui_evt.y_pos, click_distance, obj->getWorldBoundingBox());
                }
                break;
            }
            case ControllerUtil::INPUT_TYPE::MDrag:
            {
                //provides the difference in current mouse pos. from the previous mouse pos.
                std::cout << "Mouse Drag @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
                cam_yaw = ui_evt.x_pos;
                cam_pitch = ui_evt.y_pos;
                break;
            }
            default:
                std::cout << "Unknown Type" << std::endl;
        };

        ControllerUtil::print_input_type(ui_evt.event_type);
    }

    camera->move(camera_direction);
    camera->yaw(Ogre::Degree(cam_yaw)*-0.2f);
    camera->pitch(Ogre::Degree(cam_pitch)*-0.2f);
}

#endif
