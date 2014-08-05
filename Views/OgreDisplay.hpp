#ifndef TD_OGRE_DISPLAY_HPP
#define TD_OGRE_DISPLAY_HPP

#include "GameBackground.hpp"
#include "ControllerUtil.hpp"
#include "Controller.hpp"

#include "TowerDefense.hpp"
#include "util/Types.hpp"

#include <opencv2/opencv.hpp>
#include <OGRE/Ogre.h>

#include <iostream>
#include <memory>
#include <chrono>
#include <string>
#include <atomic>


class OgreDisplay
{
public:
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

        tower_build_evtqueue = nullptr;
    }

	void start_display();

    void place_tower(TowerModel* selected_tower, const std::string& tower_name, Ogre::Vector3 map_coord_offsets, Ogre::Vector3 world_coord_offsets);
    void register_input_controller(Controller* controller)
    {
        const std::string id {"ThisShouldBeSomethingMeaningful"};
        controller->register_input_listener(id, input_events.get());
    }
    
    /*
    void register_model(TowerDefense<OgreDisplay>* model)
    { td_model = model; }
    */

    void register_tower_build_queue(std::shared_ptr<UserTowerEvents::EventQueueType<UserTowerEvents::build_tower_event>::QType> build_queue)
    {
        tower_build_evtqueue = build_queue;
    }

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

    //assume we don't own this object
    //TowerDefense<OgreDisplay>* td_model;
    std::shared_ptr<UserTowerEvents::EventQueueType<UserTowerEvents::build_tower_event>::QType> tower_build_evtqueue;

    //the plan is to eventually have multiple threads running, so making this
    //atomic ahead of time (although this might change in the future...)
    std::atomic<bool> close_display;
};

#endif
