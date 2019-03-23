/* GameGUI.hpp -- part of the DietyTD Views subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */


#ifndef TD_VIEWS_UI_GAME_GUI_HPP
#define TD_VIEWS_UI_GAME_GUI_HPP

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>
#include <OIS/OIS.h>

#include <OGRE/Ogre.h>

#include "shared/common_information.hpp"
#include "shared/Player.hpp"
#include "shared/PlayerInventory.hpp"

#include "TowerUI.hpp"

/*
//TODO: consolidate this and the OgreDisplay so we only have 1 framelistener on the View side of things
class UIFrameListener : public Ogre::FrameListener, public Ogre::WindowEventListener
{
public:
    
    UIFrameListener (Ogre::RenderWindow* rwin, Ogre::Camera* cam, CEGUI::Renderer* renderer)
         
    {}

private:    
    CEGUI::Renderer* gui_renderer;

};
*/

/*
 * NOTE: this has to be initialized after the OgreDisplay is initialized (since CEGUI needs an Ogre 
 *       RenderWindow + general setup stuff to get started)
 */
class GameGUI
{
public:
    GameGUI(Ogre::RenderWindow* rwindow);
	~GameGUI();

	//Q: how involved do we want the inputs to be? i.e. I could register GUI handlers for key down
    //and key up events -- at the moment I'm just doing key down, but that doesn't seem sufficient 
    //in the long run. 
    template <class Controller>
	void register_controller(Controller* controller)
	{
        //this is where we set up the GUI event handlers
        std::function<void(OIS::KeyEvent, bool)> gui_keyhandler = [](OIS::KeyEvent key_arg, bool key_pressed)
        {
            CEGUI::GUIContext& context = CEGUI::System::getSingleton().getDefaultGUIContext();
            if(key_pressed) {
                context.injectKeyDown((CEGUI::Key::Scan)key_arg.key);
                context.injectChar((CEGUI::Key::Scan)key_arg.text);
            } else {
                context.injectKeyUp((CEGUI::Key::Scan)key_arg.key);
            }
        };

        auto cvt_mouseevts = [](OIS::MouseButtonID button_id) {
            switch (button_id)
            {
                case OIS::MB_Left:
                    return CEGUI::LeftButton;
                case OIS::MB_Right:
                    return CEGUI::RightButton;
                case OIS::MB_Middle:
                    return CEGUI::MiddleButton;
                default:
                    return CEGUI::LeftButton;
            }
        };

        //mouse_state:
        //  0: movement, 1: button down, 2: button up 
        std::function<void(OIS::MouseEvent, OIS::MouseButtonID, int)> gui_mousehandler = [cvt_mouseevts](OIS::MouseEvent mouse_evt, OIS::MouseButtonID mouse_id, int mouse_state)
        {
            //std::cout << "mouse GUI event handler" << std::endl;
            auto& context = CEGUI::System::getSingleton().getDefaultGUIContext();
            //context.injectMouseMove(mouse_evt.state.X.rel, mouse_evt.state.Y.rel);
            context.injectMousePosition(mouse_evt.state.X.abs, mouse_evt.state.Y.abs);

            //NOTE: the Z-axis is the scroll wheel
            if (mouse_evt.state.Z.rel) {
                std::cout << "we're scrollllling" << std::endl;
                context.injectMouseWheelChange(mouse_evt.state.Z.rel / 120.0f);
            }

            if(mouse_state == 1) {
               context.injectMouseButtonDown (cvt_mouseevts(mouse_id));
            } else if (mouse_state == 2){
               context.injectMouseButtonUp (cvt_mouseevts(mouse_id));
            }
        }; 
        controller->register_gui_listener(std::move(gui_keyhandler), std::move(gui_mousehandler));
	}

    void register_shared_towerinfo(std::shared_ptr<GameInformation<CommonTowerInformation, TDPlayerInformation>> shared_info)
	{
        shared_gamestate_info = shared_info;

		//propogate the pointer to other sub-UIs as well
        tower_modify_ui->register_shared_towerinfo(shared_info);
	}

	inline void update_gamestate_info(const TDPlayerInformation& info) 
	{
        set_lives(info.get_num_lives());
        set_gold(info.get_num_gold());
        set_essence(info.get_num_essence());
	}

    //--------------------------------------------------------------------------------------------------------------------------------------
    //NOTE: these are sort of redundant, as we now have the shared_gamestate_info as a member variable. So, we don't need to have
    //external input regarding the player info updates. How to handle this?
    inline void update_inventory_info(const TDPlayerInformation& info)
    {
		//update the inventory state based on the new player snapshot when the round ends and the
        //dropped items are added to the inventory.
		tower_modify_ui->update_inventory(info.get_inventory_state());

    }

    inline std::pair<bool, TDPlayerInformation> get_inventory_updates() const
    {
        //check if we made any changes to the player inventory
        //.... so far the only place that would happen is in the tower UI. In theory we will eventually
        //add more, different UIs which could also affect the player inventory. In which case, we'll have
        //to aggregate the inventory changes and keep a consistent version.... somewhere, s.t. when a UI
        //opens it'll get the current inventory, and when it closes we'll update our frontend master copy
        //of the inventory with any changes that may have been made. Or something like that

        auto ui_snapshot = tower_modify_ui->get_inventory_updates();

        //TODO: this is where we would do some sort of check to see if the inventory has changed?
        //....
        //

		TDPlayerInformation ui_playerinfo (num_lives_, num_essence_, num_gold_);

		//TODO: need to merge the inventory data here?
		//ui_playerinfo.add_item(ui_snapshot);

        bool inventory_updated = true;
        return std::make_pair(inventory_updated, ui_playerinfo);
    }
    //--------------------------------------------------------------------------------------------------------------------------------------

    inline void close_all_windows()
    {
        if(tower_modify_ui->is_showing()) {
            tower_modify_ui->close_all_windows();
        }

        //TODO: close any other open windows
    }

	//called by the main frontend class in response to user input. This is where we make the TowerUpgrade UI 
	//the active window
    void handle_tower_upgrade(uint32_t active_tower_ID)
    {
        tower_modify_ui->activate_towerUI(active_tower_ID);
    }

    void display_information(const std::string& base_stats, const std::string& current_stats, const std::string& unit_info);

    //returns whether a subwindow is showing. Is used to disable certain things in the frontend (i.e. panning the camera)
    inline bool is_subwindow_showing() const 
    {
        //TODO: eventually (as we add other UI windows) there would be others to check
        bool tower_ui_flag = tower_modify_ui->is_showing();

        return tower_ui_flag;
    }

    inline bool has_pending_tower_modifications() const 
    {
        return tower_modify_ui->active_tower_mods.size() > 0;
    }

    //kind of weird to return a pointer to a vector... maybe I should just return an iterator? I want to modify the
    //TowerUI vector however. or... I could just return a copy of the vector, and clear the one in the TowerUI?
    //this assumes that the caller will handle all of the events... but that seems like a safe bet anyways
    inline std::vector<TowerUpgradeUI::tower_modifyinfo> get_pending_tower_modifications() const 
    {
        auto active_mods = tower_modify_ui->active_tower_mods;

        tower_modify_ui->active_tower_mods.clear();
        return active_mods;
    }

private:	
	int num_lives_;
	int num_gold_;
	int num_essence_;

	void initialize_mainUI();

    void setup_animations();
	void set_lives(int amount);
	void set_gold(int amount);
	void set_essence(int amount);

	CEGUI::OgreRenderer* gui_renderer;
    CEGUI::System* gui_sys;
    CEGUI::Window* gui_window;

    CEGUI::RenderTarget *gui_rendertarget;
    CEGUI::GUIContext *gui_context;

	std::unique_ptr<TowerUpgradeUI> tower_modify_ui;
    std::shared_ptr<GameInformation<CommonTowerInformation, TDPlayerInformation>> shared_gamestate_info;
};




#endif
