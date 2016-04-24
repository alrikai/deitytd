/* OgreDisplay.hpp -- part of the DietyTD Views subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_OGRE_DISPLAY_HPP
#define TD_OGRE_DISPLAY_HPP

#include "ControllerUtil.hpp"
#include "Controller.hpp"

#include "GameBackground.hpp"
#include "ViewEventTypes.hpp"
#include "ViewUtil.hpp"
#include "UI/GameGUI.hpp"

#include "TowerDefense.hpp"
#include "util/Types.hpp"
#include "shared/common_information.hpp"

#include <opencv2/opencv.hpp>
#include <OGRE/Ogre.h>

#include <iostream>
#include <memory>
#include <chrono>
#include <string>
#include <atomic>

//might want to consider breaking this class up into smaller ones reeeal soon

struct MoveableObject
{
  MoveableObject(const Ogre::Vector3& dest, Ogre::SceneNode* snode)
    : obj_destination(dest), obj_snode(snode)
  {
      move_destination = obj_destination;
      x_delta = 0.0f;
      y_delta = 0.0f;
      is_active = false;
  }

  virtual void update_movedest(const Ogre::Vector3& dest_pos, const double time_duration)
  {
      move_destination = dest_pos;
      move_duration = time_duration;

      auto pos_delta = dest_pos - obj_snode->getPosition();
      x_delta = pos_delta.x / time_duration;
      y_delta = pos_delta.y / time_duration;

      is_active = true;

        /*
        /////////////////////////////////////////////////////////////////////////////////////////
        //for testing: seeif we have a situation where the destination is further than the current pos.
        //this is just for debugging to identify the problem
        Ogre::Vector3 map_hsz {102, 76.5, 0};
        auto attack_pos = attack_snode->getPosition() + map_hsz;
        auto dest_fullpos = dest_pos + map_hsz;
        auto full_dest = attack_destination + map_hsz;
        //get the distance from the final destination. In theory, the  move_destination should be closer than the current position
        float src_dist = std::sqrt((full_dest.x - attack_pos.x) * (full_dest.x - attack_pos.x) + 
                                   (full_dest.y - attack_pos.y) * (full_dest.y - attack_pos.y));
        float dest_dist = std::sqrt((full_dest.x - dest_fullpos.x) * (full_dest.x - dest_fullpos.x) + 
                                   (full_dest.y - dest_fullpos.y) * (full_dest.y - dest_fullpos.y));
        */

  }
 

  //time_elapsed is in ms
  virtual void update_position(const float time_elapsed)
  {
      if(is_active)
      {
          auto current_pos = obj_snode->getPosition();
          current_pos.x += time_elapsed * x_delta;
          current_pos.y += time_elapsed * y_delta;
          obj_snode->setPosition(current_pos);

          move_duration -= time_elapsed;
          //std::cout << origin_id << " -- time delta: " << move_duration << std::endl;
          is_active = (move_duration >= 0);
          if(!is_active)
            std::cout << "Update Position Turning off -- over time limit!" << std::endl; 
      }
  }

  Ogre::Vector3 obj_destination;
  Ogre::Vector3 move_destination;
  float x_delta;
  float y_delta;
  float move_duration;
  bool is_active;

  Ogre::SceneNode* obj_snode;
};

struct TowerAttackAnimation : MoveableObject
{
    TowerAttackAnimation(const std::string id, const Ogre::Vector3& dest, Ogre::SceneNode* snode)
      : MoveableObject(dest, snode), origin_id(id)
    {}

    //the tower id from whence the attack was spawned
    const std::string origin_id;
};

template <class BackendType>
struct TDMapInformation
{
    //reserved as the 'no identifier' identifier
    static constexpr uint32_t INVALID_ID = 0x0;

    TDMapInformation()
    {
        //initialize the tower mapping to have no valid IDs
        for (int row = 0; row < BackendType::MAP_NUMTILES_HEIGHT; row++) {
            for (int col = 0; col < BackendType::MAP_NUMTILES_WIDTH; col++) {
                tower_ID_mapping[row][col] = INVALID_ID;
            }
        }
    }

    //TODO: need to see which coordinated are best/most convenient for storage purposes here...
    void add_tower_ID(float xnorm_coord, float ynorm_coord, const uint32_t tower_ID)
    {
        assert(xnorm_coord >= 0 && xnorm_coord <= 1);
        assert(ynorm_coord >= 0 && ynorm_coord <= 1);

        float xtile = xnorm_coord * BackendType::MAP_NUMTILES_WIDTH;
        float ytile = ynorm_coord * BackendType::MAP_NUMTILES_HEIGHT;
        int tile_col = std::floor(xtile);
        int tile_row = std::floor(ytile);

        assert(tile_row < BackendType::MAP_NUMTILES_HEIGHT);
        assert(tile_col < BackendType::MAP_NUMTILES_WIDTH);

        //TODO: not really sure what to do if this ever happens... might be exception throwin' time
        if(tower_ID_mapping[tile_row][tile_col]) {
            std::cerr << "ERROR -- TDMapInformation already has ID " << tower_ID_mapping[tile_row][tile_col] << " at " << tile_row << ", " << tile_col << std::endl;
        }

        tower_ID_mapping[tile_row][tile_col] = tower_ID;
    }

    //returns true if there's a tower, false otherwise
    bool check_for_tower(float xnorm_coord, float ynorm_coord)
    {
        return (get_tower_ID(xnorm_coord, ynorm_coord) != 0);
    }

    uint32_t get_tower_ID (float xnorm_coord, float ynorm_coord)
    {
        assert(xnorm_coord >= 0 && xnorm_coord <= 1);
        assert(ynorm_coord >= 0 && ynorm_coord <= 1);

        //map the normalized coordinates to the map tile (Q: will there be rounding issues here?)
        float xtile = xnorm_coord * BackendType::MAP_NUMTILES_WIDTH;
        float ytile = ynorm_coord * BackendType::MAP_NUMTILES_HEIGHT;
        int tile_col = std::floor(xtile);
        int tile_row = std::floor(ytile);

        assert(tile_row < BackendType::MAP_NUMTILES_HEIGHT);
        assert(tile_col < BackendType::MAP_NUMTILES_WIDTH);
        
        return tower_ID_mapping[tile_row][tile_col];
    }

    //for faster lookup between user clicks and getting the constrituent tower ID
    static constexpr double UNITS_PER_TILE_H = 1.0 / BackendType::MAP_NUMTILES_HEIGHT;
    static constexpr double UNITS_PER_TILE_W = 1.0 / BackendType::MAP_NUMTILES_WIDTH;
    using TowerMapContainer = std::array<std::array<uint32_t, BackendType::MAP_NUMTILES_WIDTH>, BackendType::MAP_NUMTILES_HEIGHT>;
    TowerMapContainer tower_ID_mapping; 
};

//-----------------------------------------------------------------------------------

template <class BackendType>
class OgreDisplay : public Ogre::FrameListener, public Ogre::WindowEventListener
{
public:
    using TowerEventQueueType = typename UserTowerEvents::EventQueueType<UserTowerEvents::tower_event<BackendType>>::QType; 
/*
    using RenderEventQueueType = RenderEvents::MakeTowerQType; 
    using RenderAttackEventQueueType = RenderEvents::MakeAttackQType;
    using RenderAttackMoveEventQueueType = RenderEvents::MakeAttackMoveQType;
*/
    OgreDisplay()
        : root (new Ogre::Root(plugins_cfg_filename)), cam_rotate(0.10f), cam_move(10.0f),
        background(nullptr), td_event_queue(nullptr), game_events(nullptr), gui(nullptr), close_display(false), 
        towercount_ID(TDMapInformation<BackendType>::INVALID_ID+0x1)
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

        Ogre::Root::getSingletonPtr()->addFrameListener(this);
    }

    void start_display();

    void place_tower(TowerModel* selected_tower, const uint32_t tower_ID, const std::string& tower_name, const Ogre::AxisAlignedBox& map_box,
                     Ogre::Vector3 map_coord_offsets, Ogre::Vector3 world_coord_offsets);
    void place_mob(const CharacterModels::ModelIDs id, const std::string& mob_name, const Ogre::AxisAlignedBox& map_box, 
                                         Ogre::Vector3 map_coord_offsets);
    void register_input_controller(Controller* controller)
    {
        const std::string id {"OgreDisplayView"};
        controller->register_input_listener(id, input_events.get());
        gui->register_controller(controller);        
    }

    //for enqueueing frontend --> backend events
    void register_tower_eventqueue(TowerEventQueueType* tevt_queue)
    {
        td_event_queue = tevt_queue;
    }
    
    //for processing backend --> frontend events
    void register_backend_eventqueue(ViewEvents* events)
    {
        game_events = events;
    }

	void register_shared_info(std::shared_ptr<GameInformation<CommonTowerInformation>> shared_info)
	{
        shared_tower_info = shared_info;
        gui->register_shared_towerinfor(shared_tower_info);
	}

    Ogre::Root* get_root() const
    { return root.get(); }
    Ogre::RenderWindow* get_render_window() const 
    { return render_window; }
    Ogre::SceneManager* get_scene_mgmt() const
    { return scene_mgmt; }

    void draw_maptiles(const int num_col_tiles, const int num_row_tiles)
    { 
        background->draw_tiles(num_col_tiles, num_row_tiles); 
    }

    //Adjust mouse clipping area
    //void windowResized(Ogre::RenderWindow* rw) override;
    //Unattach OIS before window shutdown (very important under Linux)
    //void windowClosed(Ogre::RenderWindow* rw) override;
    bool frameRenderingQueued(const Ogre::FrameEvent& evt) override;

    void update_gameinfo();

private:        
    bool ogre_setup();
    void gui_setup();

    void setup_camera(); 
    void setup_background();
    void handle_user_input();

    void get_mapcoords(const std::vector<float>& world_position, float& x_coord, float& y_coord, const Ogre::AxisAlignedBox& map_box);
    void generate_tower(const float x_coord, const float y_coord, const float click_distance, const Ogre::AxisAlignedBox& mapobj);
    void generate_information_request(std::vector<float>&& world_position);

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

    //----------------------------------------
    std::unique_ptr<GameGUI> gui;
    //TODO: used to update the GUI passive info to reflect the current game state 
    GameStateInformation game_state_info;
    //----------------------------------------

    std::unique_ptr<GameBackground> background;
    std::unique_ptr<ControllerUtil::ControllerBufferType> input_events;

    //the tower event queues, for frontend --> backend and backend --> frontend communication
    //-- these are both owned by the TowerDefense class, hence the raw ponters
    TowerEventQueueType* td_event_queue;
    ViewEvents* game_events;
    std::shared_ptr<GameInformation<CommonTowerInformation>> shared_tower_info;

    //the plan is to eventually have multiple threads running, so making this
    //atomic ahead of time (although this might change in the future...)
    std::atomic<bool> close_display;
    uint32_t towercount_ID;
    TDMapInformation<BackendType> tower_mapinfo;

    //holds the state of the current selection of the user (or nullptr if none selected)
    //this will presumably either be towers or mobs -- wouldn't make sense to be able to click attacks
    Ogre::MovableObject* current_selection;

    //note: ogre manages the animation lifetimes 
    std::map<std::string, Ogre::AnimationState*> tower_animations;
    std::map<std::string, Ogre::AnimationState*> mob_animations;

    std::map<std::string, TowerAttackAnimation> tower_attacks;
    std::map<std::string, MoveableObject> live_mobs;
};


template <class BackendType>
const std::string OgreDisplay<BackendType>::resource_cfg_filename {"resources.cfg"};

template <class BackendType>
const std::string OgreDisplay<BackendType>::plugins_cfg_filename {"plugins.cfg"};


//this is the main rendering loop
template <class BackendType>
void OgreDisplay<BackendType>::start_display()
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////
    auto tbuild_evt_fcn = [this](std::unique_ptr<RenderEvents::create_tower> render_evt)
    {
        Ogre::Vector3 t_map_offsets {render_evt->t_map_offsets[0], render_evt->t_map_offsets[1], render_evt->t_map_offsets[2]};
        Ogre::Vector3 t_world_offsets {render_evt->t_world_offsets[0], render_evt->t_world_offsets[1], render_evt->t_world_offsets[2]};                 

        //NOTE: this is the last piece of the puzzle. Should be the GameMap (GameBGMap), but I formally would get it from a scene query.
        Ogre::AxisAlignedBox map_box = this->background->get_map_aab();
        this->place_tower(render_evt->t_model.get(), render_evt->t_ID, render_evt->t_name, map_box, t_map_offsets, t_world_offsets);
    };

    auto atkbuild_evt_fcn = [this](std::unique_ptr<RenderEvents::create_attack> render_evt)
    {
        auto origin_tname = render_evt->origin_tname;
        Ogre::Vector3 origin = scene_mgmt->getEntity(origin_tname)->getParentSceneNode()->getPosition();
        Ogre::Entity* tower_atk = scene_mgmt->createEntity(render_evt->name, Ogre::SceneManager::PT_SPHERE);
        tower_atk->setMaterialName("Examples/Chrome");
        tower_atk->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
        auto child_node = root_node->createChildSceneNode(render_evt->name);

        //scale the attack to a more sensible size...
        Ogre::Vector3 scale_vec(1/25.0f,1/25.0f, 1/25.0f);
        child_node->setScale(scale_vec);
        child_node->setPosition(origin.x, origin.y, origin.z);
        child_node->attachObject(tower_atk);  

        TowerAttackAnimation attack_anim(origin_tname, origin, child_node);
        tower_attacks.insert(std::make_pair(render_evt->name, attack_anim));
    };

    auto atkmove_evt_fcn = [this](std::unique_ptr<RenderEvents::move_attack> render_evt)
    {
        const double time_duration = render_evt->duration;
        auto attack_id = render_evt->name;
        auto tower_it = tower_attacks.find(attack_id);
        if(tower_it != tower_attacks.end())
        {
            Ogre::AxisAlignedBox map_box = this->background->get_map_aab();
            auto map_dimensions = map_box.getSize();
            auto parent_snode = scene_mgmt->getEntity(attack_id)->getParentSceneNode();
            Ogre::Vector3 movement {render_evt->delta[0], render_evt->delta[1], parent_snode->getPosition().z};
            //the position for the attack node to be at after the time_duration elapses
            movement = map_dimensions * movement - map_box.getHalfSize();
            tower_it->second.update_movedest(movement, time_duration);
        }
    };

    auto atkremove_evt_fcn = [this](std::unique_ptr<RenderEvents::remove_attack> render_evt)
    {
        auto attack_id = render_evt->name;
        auto tower_it = tower_attacks.find(attack_id);
        if(tower_it != tower_attacks.end())
            tower_attacks.erase(tower_it);

        Ogre::SceneNode* t_scenenode = scene_mgmt->getEntity(attack_id)->getParentSceneNode(); 
        OgreUtil::nuke_scenenode(t_scenenode);
    };

    auto mbuild_evt_fcn = [this](std::unique_ptr<RenderEvents::create_mob> render_evt)
    {
        Ogre::Vector3 m_map_offsets {render_evt->m_map_offsets[0], render_evt->m_map_offsets[1], render_evt->m_map_offsets[2]};

        std::cout << "@FRONT -- making " << render_evt->m_name << std::endl;

        //NOTE: this is the last piece of the puzzle. Should be the GameMap (GameBGMap), but I formally would get it from a scene query.
        Ogre::AxisAlignedBox map_box = this->background->get_map_aab();
        this->place_mob(render_evt->model_id, render_evt->m_name, map_box, m_map_offsets);
    };

    auto mobmove_evt_fcn = [this](std::unique_ptr<RenderEvents::move_mob> render_evt)
    {
        const double time_duration = render_evt->duration;
        auto mob_name = render_evt->name;
        auto mob_it = live_mobs.find(mob_name);
        if(mob_it != live_mobs.end())
        {
          Ogre::AxisAlignedBox map_box = this->background->get_map_aab();
          auto map_dimensions = map_box.getSize();
          auto parent_snode = scene_mgmt->getEntity(mob_name)->getParentSceneNode();
          Ogre::Vector3 movement {render_evt->delta[0], render_evt->delta[1], parent_snode->getPosition().z};
          //the position for the mob node to be at after the time_duration elapses
          movement = map_dimensions * movement - map_box.getHalfSize();
          mob_it->second.update_movedest(movement, time_duration);
        }
    };

    auto mobremove_evt_fcn = [this](std::unique_ptr<RenderEvents::remove_mob> render_evt)
    {
        auto mob_name = render_evt->name;
    
        std::cout << "@FRONT -- removing " << mob_name << std::endl;

        auto mob_it = live_mobs.find(mob_name);
        if(mob_it != live_mobs.end())
        {
          live_mobs.erase(mob_it);
        }

        //TODO: check if we are deleteing the correct scenenode. When we execute this, we end up
        //hanging forever at renderOneFrame down below, so presumably something is getting nuked that shouldnt be
        auto m_scenenode = scene_mgmt->getEntity(mob_name)->getParentSceneNode(); 
        OgreUtil::nuke_scenenode(m_scenenode);
    };

            
    auto unitinfo_evt_fcn = [this](std::unique_ptr<RenderEvents::unit_information> info_evt)
    {
        //TODO: reeeeeally need to re-work how the GUI display will be arranged
        info_evt->base_stats = current_selection->getName();
        gui->display_information(info_evt->base_stats, info_evt->current_stats, info_evt->information);
    };

///////////////////////////////////////////////////////////////////////////////////////////////////////

    //10 FPS is the bare minimum
    constexpr int max_ms_perframe = 100;

    double time_elapsed = 0;

    //have the game commit suicide after 600 seconds
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
        background->draw_background();

        /////////////////////////////////////////////////////////////////////////////////////////////

        game_events->apply_towerbuild_events(tbuild_evt_fcn);
        game_events->apply_attackbuild_events(atkbuild_evt_fcn);
        game_events->apply_attackmove_events(atkmove_evt_fcn);
        game_events->apply_attackremove_events(atkremove_evt_fcn);
 
        game_events->apply_mobbuild_events(mbuild_evt_fcn);
        game_events->apply_mobmove_events(mobmove_evt_fcn);
        game_events->apply_mobremove_events(mobremove_evt_fcn);

        //give request for user selection information
        game_events->apply_unitinfo_events(unitinfo_evt_fcn);        
        
        /////////////////////////////////////////////////////////////////////////////////////////////
     
        auto end_time = std::chrono::high_resolution_clock::now(); 
        std::chrono::duration<double, std::milli> time_duration (end_time - start_time);
        time_elapsed = time_duration.count();
    } while(time_elapsed < TOTAL_TIME && !close_display.load());

    //at this point we have to kill the background generator as well
    //...
 }


template <class BackendType>
void OgreDisplay<BackendType>::setup_camera()
{
    camera = scene_mgmt->createCamera("MinimalCamera");
    camera->setNearClipDistance(5);
    camera->setFarClipDistance(6000);
    //have a 4:3 aspect ratio, looking back along the Z-axis (should we do Y-axis instead?) 
    camera->setAspectRatio(Ogre::Real(4.0f/3.0f));
    camera->setPosition(Ogre::Vector3(-20,0,300)); 
    camera->lookAt(Ogre::Vector3(0,0,0));
}


template <class BackendType>
bool OgreDisplay<BackendType>::ogre_setup()
{
    //load resources
    view_detail::load_resources(resource_cfg_filename);
    
    //configure the system
    if(!root->restoreConfig()) {
        if(!root->showConfigDialog()) {
            return false;
        }
    }

    render_window = root->initialise(true, "DietyTD");
    scene_mgmt = root->createSceneManager("OctreeSceneManager");
    root_node = scene_mgmt->getRootSceneNode();
	
	gui_setup();

    return true;
}


template <class BackendType>
void OgreDisplay<BackendType>::gui_setup()
{
    gui = std::unique_ptr<GameGUI> (new GameGUI (render_window));
}


//Q: who would call this?
template <class BackendType>
void OgreDisplay<BackendType>::update_gameinfo() 
{
    //TODO: populate this somehow?? Should this spawn a request to something (?) that has the info,
    //or should it just periodically update the info, or should it be registered as some sort of event
    //listener that updates when the backend sends new info?
    GameStateInformation placeholder_info;
    gui->update_gamestate_info(placeholder_info);
}


template <class BackendType>
void OgreDisplay<BackendType>::get_mapcoords(const std::vector<float>& world_position, float& xnorm_coord, float& ynorm_coord, const Ogre::AxisAlignedBox& map_box)
{
    Ogre::Vector3 map_coord_mapping = map_box.getSize(); 
    xnorm_coord = 0;
    ynorm_coord = 0;

    auto world_click_x = world_position[0];
    auto world_click_y = world_position[1];

    if(world_click_x < 0)
        xnorm_coord = 0.5f - std::abs(world_click_x / map_coord_mapping.x);
    else
        xnorm_coord = 0.5f + std::abs(world_click_x / map_coord_mapping.x);

    if(world_click_y < 0)
        ynorm_coord = 0.5f - std::abs(world_click_y / map_coord_mapping.y);
    else
        ynorm_coord = 0.5f + std::abs(world_click_y / map_coord_mapping.y);
}

template <class BackendType>
void OgreDisplay<BackendType>::generate_information_request(std::vector<float>&& world_position)
{
    float xnorm_coord, ynorm_coord;
    get_mapcoords(world_position, xnorm_coord, ynorm_coord, this->background->get_map_aab());

    using tower_evt_t = UserTowerEvents::print_tower_event<BackendType>;
    std::unique_ptr<UserTowerEvents::tower_event<BackendType>> td_evt = std::unique_ptr<tower_evt_t> (new tower_evt_t(ynorm_coord, xnorm_coord));
    td_event_queue->push(std::move(td_evt));            
}

template <class BackendType>
void OgreDisplay<BackendType>::generate_tower(const float x_coord, const float y_coord, const float click_distance, const Ogre::AxisAlignedBox& map_box)
{
    Ogre::Ray ray = camera->getCameraToViewportRay(x_coord/view_port->getActualWidth(), y_coord/view_port->getActualHeight());
    auto world_click = ray.getPoint(click_distance);

    std::cout << "World Click: " << world_click << " vs " << map_box << std::endl;
    //convert the click to map coordinates
    Ogre::Vector3 map_coord_mapping = map_box.getSize(); 

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

    //this would come from the GUI (someday we'll have a GUI...)
    const int tier = 1;

    //make a unique ID for the tower being generated. This will be common to the front and backend
    const uint32_t tower_ID = towercount_ID;
    towercount_ID += 1;
    std::cout << "generating tower ID " << tower_ID << std::endl;

    //tell the backend that the user built a tower
    using tower_evt_t = UserTowerEvents::build_tower_event<BackendType>;
    std::unique_ptr<UserTowerEvents::tower_event<BackendType>> td_evt = 
        std::unique_ptr<tower_evt_t> (new tower_evt_t(tower_ID, tier, norm_mapcoords_row, norm_mapcoords_col));
    td_event_queue->push(std::move(td_evt));
}

template <class BackendType>
void OgreDisplay<BackendType>::place_tower(TowerModel* selected_tower, const uint32_t tower_ID, const std::string& tower_name, const Ogre::AxisAlignedBox& map_box, 
                                           Ogre::Vector3 map_coord_offsets, Ogre::Vector3 world_coord_offsets)
{
    std::cout << "TileCenter Offset: [" << map_coord_offsets.x << ", " << map_coord_offsets.y << "] --> ID: " << tower_ID << std::endl;

    //TODO: want the scale to be based on a few factors, such as the resolution, map size, and fractal dimensions
    const float tower_scale = 1.0f/4.0f;

    //store the tower IDs here 
    tower_mapinfo.add_tower_ID(map_coord_offsets.x, map_coord_offsets.y, tower_ID);

	auto tinfo = shared_tower_info->get_towerinfo(tower_ID);
	std::cout << tinfo.tower_name << std::endl;

    //NOTE: we want to have the tower ABOVE the map -- thus, its z coordinate has to be non-zero 
    const Ogre::Vector3 target_location { map_box.getHalfSize().x * (2 * (map_coord_offsets.x - 0.5f)), 
                                          map_box.getHalfSize().y * (2 * (map_coord_offsets.y - 0.5f)),
                                          tower_scale * world_coord_offsets.z};    

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
    auto tower_snode = root_node->createChildSceneNode(tower_name);
    tower_snode->attachObject(tower_entity);   
     
    //NOTE: position is (x, y, z)
    tower_snode->setPosition(target_location.x, target_location.y, target_location.z);
    tower_snode->scale(tower_scale, tower_scale, tower_scale);
    
    tower_snode->showBoundingBox(!true);

    /*
    std::vector<std::string> particle_types
    {
    "Examples/Aureola",
    "Examples/GreenyNimbus",
    "Examples/Swarm",
    "Examples/PurpleFountain"
    };
    */
    const std::vector<std::string> particle_types
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
    tower_snode->attachObject(particle_node);


    ///////////////////////////////////////////
    //attempt at animation....
    ///////////////////////////////////////////

    const float step = 10.0f;
    const float duration = 4.0f * step;
    const std::string animation_id {tower_name + "_animation"};
    Ogre::Animation* tower_animation = scene_mgmt->createAnimation(animation_id, duration);
    tower_animation->setInterpolationMode(Ogre::Animation::IM_SPLINE);
    Ogre::NodeAnimationTrack* tower_track = tower_animation->createNodeTrack(0, tower_snode);

    //make some animation keyframes
    Ogre::TransformKeyFrame* tower_keyf;
    
    //t0
    tower_keyf = tower_track->createNodeKeyFrame(0);
    tower_keyf->setTranslate(Ogre::Vector3(target_location.x, target_location.y, target_location.z));
    tower_keyf->setScale(Ogre::Vector3(tower_scale, tower_scale, tower_scale));
    //t1
    tower_keyf = tower_track->createNodeKeyFrame(1.0f * step);
    tower_keyf->setTranslate(Ogre::Vector3(target_location.x, target_location.y, target_location.z + 5));
    tower_keyf->setScale(Ogre::Vector3(tower_scale, tower_scale, tower_scale));
    //etc...
    tower_keyf = tower_track->createNodeKeyFrame(2.0f * step);
    tower_keyf->setTranslate(Ogre::Vector3(target_location.x, target_location.y, target_location.z - 5));
    tower_keyf->setScale(Ogre::Vector3(tower_scale, tower_scale, tower_scale));

    tower_keyf = tower_track->createNodeKeyFrame(3.0f * step);
    tower_keyf->setTranslate(Ogre::Vector3(target_location.x, target_location.y, target_location.z + 5));
    tower_keyf->setScale(Ogre::Vector3(tower_scale, tower_scale, tower_scale));

    tower_keyf = tower_track->createNodeKeyFrame(4.0f * step);
    tower_keyf->setTranslate(Ogre::Vector3(target_location.x, target_location.y, target_location.z));
    tower_keyf->setScale(Ogre::Vector3(tower_scale, tower_scale, tower_scale));

    auto tower_animation_mgmt = scene_mgmt->createAnimationState(animation_id);
    tower_animation_mgmt->setEnabled(true);
    tower_animation_mgmt->setLoop(true);
    tower_animations.emplace(std::make_pair(animation_id, tower_animation_mgmt));
}

template <class BackendType>
void OgreDisplay<BackendType>::place_mob(const CharacterModels::ModelIDs id, const std::string& mob_name, const Ogre::AxisAlignedBox& map_box, 
                                         Ogre::Vector3 map_coord_offsets)
{
  //NOTE: we want to have the tower ABOVE the map -- thus, its z coordinate has to be non-zero 
  const Ogre::Vector3 target_location { map_box.getHalfSize().x * (2 * (map_coord_offsets.x - 0.5f)), 
                                        map_box.getHalfSize().y * (2 * (map_coord_offsets.y - 0.5f)),
                                        1};    

  const auto model_id = CharacterModels::id_names.at(static_cast<int>(id));
  auto model_ent = scene_mgmt->createEntity(mob_name, model_id + ".mesh");
  model_ent->setRenderQueueGroup(Ogre::RENDER_QUEUE_MAIN);
  
  auto mob_snode = root_node->createChildSceneNode(mob_name);
  mob_snode->attachObject(model_ent);   

  //OgreUtil::load_model(mob_snode, id, mob_name);

  //NOTE: position is (x, y, z)
  constexpr float mob_scale = 1.0f;
  mob_snode->setPosition(target_location.x, target_location.y, target_location.z);
  mob_snode->scale(mob_scale, mob_scale, mob_scale);
  mob_snode->showBoundingBox(true);

  MoveableObject mob_anim(target_location, mob_snode);
  live_mobs.insert(std::make_pair(mob_name, mob_anim));

  /*
  //NOTE: if we have things specific to the animation, where should we do those?
  //TODO: figure out how generic we need this to be. Every character will have some state machine
  //that determines how it acts / how it appears (i.e. idle, active, stunned, dead, etc)
  auto base_animation = model_ent->getAnimationState("IdleBase");
  auto top_animation = model_ent->getAnimationState("IdleTop");
  base_animation->setLoop(true);
  top_animation->setLoop(true);
  base_animation->setEnabled(true);
  top_animation->setEnabled(true);

  const std::string bot_animation_id {mob_name + "_animation_B"};
  mob_animations.emplace(std::make_pair(bot_animation_id, base_animation));
  const std::string top_animation_id {mob_name + "_animation_T"};
  mob_animations.emplace(std::make_pair(top_animation_id, top_animation));
  */
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
            case ControllerUtil::INPUT_TYPE::C:
            {
                //if we have a tower selected, need to fire off an event to the GUI to show the tower upgrade window
                if(current_selection)
                {
                    //TODO: verify that a TOWER (not a monster) is selected

                    //NOTE: we will need a lot more infrastructure here. In order to upgrade the tower, we need to know 
                    //the information about the tower, plus we need some other data (e.g. #word slots, etc).
                    //which will likely necessitate another information request to the backend, which will be something new
                    //(but important) -- doing on-demand information requests between frontend and backend (or at least, 
                    //handling the communication latency in a sensible manner). Simplest would just be to wait on getting
                    //the response from the backend, but then we'll be blocking the frontend
 
                    std::cout << "Combining " << current_selection->getName() << " @ " << current_selection->getParentSceneNode()->getPosition() << std::endl;

                    auto selection_loc = current_selection->getParentSceneNode()->getPosition();
                    std::vector<float> selection_position {selection_loc.x, selection_loc.y, selection_loc.z};
                    float xnorm_coord, ynorm_coord;
                    get_mapcoords(selection_position, xnorm_coord, ynorm_coord, this->background->get_map_aab());
                    auto selection_ID = tower_mapinfo.get_tower_ID(xnorm_coord, ynorm_coord);
                    
                    gui->handle_tower_upgrade(selection_ID);
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
                
                //TODO: need to completely overhaul the raycasting and user selection code. More re-computation than is necessary here

                //NOTE: current_selection is set to nullptr if nothing was selected. Might want to disallow selecting the game map and certain other objects
                current_selection = view_detail::user_select(scene_mgmt, view_port, ui_evt.x_pos/width, ui_evt.y_pos/height);
                if(current_selection) {
                    //gets the position clicked in the world -- can get the map coordinates from this
                    bool is_valid = false;
                    float x_world_coord, y_world_coord, z_world_coord;
                    std::tie(is_valid, x_world_coord, y_world_coord, z_world_coord) = view_detail::get_worldclick_coords(scene_mgmt, view_port, ui_evt.x_pos, ui_evt.y_pos);
                    if(is_valid) {

                        std::vector<float> worldclick_position {x_world_coord, y_world_coord, z_world_coord};
                        float xnorm_coord, ynorm_coord;
                        get_mapcoords(worldclick_position, xnorm_coord, ynorm_coord, this->background->get_map_aab());
                        auto selection_towerID = tower_mapinfo.get_tower_ID(xnorm_coord, ynorm_coord);

                        std::cout << "Selected tID " << selection_towerID << " -- " << current_selection->getName() << " @ " << current_selection->getParentSceneNode()->getPosition() << std::endl;

                        /*
                        float xnorm_coord, ynorm_coord;
                        get_mapcoords(selection_position, xnorm_coord, ynorm_coord, this->background->get_map_aab());
                        */

                        generate_information_request(std::move(worldclick_position));
                        //TODO: we should spawn an information request to the backend for the selection and update the GUI accordingly
                        //request_selection = true;
                    }
                }
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
                //std::cout << "Mouse Drag @[" << ui_evt.x_pos << ", " << ui_evt.y_pos << "]" << std::endl;
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

//void windowResized(Ogre::RenderWindow* rw) override;
//void windowClosed(Ogre::RenderWindow* rw) override;

template <class BackendType>
bool OgreDisplay<BackendType>::frameRenderingQueued(const Ogre::FrameEvent& evt)
{
    //spin the towers
    for (auto tower_it : tower_animations) {
        tower_it.second->addTime(evt.timeSinceLastFrame);
    }
 
    for (auto mob_it : mob_animations) {
        mob_it.second->addTime(evt.timeSinceLastFrame);
    }
    
    //NOTE: evt is in seconds, so need to convert to ms
    for (auto attack_it : tower_attacks)
    {
        attack_it.second.update_position(1000.0f * evt.timeSinceLastFrame);
    }

    for (auto mob_it : live_mobs)
    {
        mob_it.second.update_position(1000.0f * evt.timeSinceLastFrame);
    }

    //TODO: determine if this is a good place for this, or if I should put it in a seperate frame
    //listener (i.e. in the controller? or in a GUI-specific frame listener?
    CEGUI::System::getSingleton().injectTimePulse(evt.timeSinceLastFrame);

    return true;
}



#endif
