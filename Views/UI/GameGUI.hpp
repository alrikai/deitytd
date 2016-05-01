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

		//next, update the inventory state based on the new player snapshot IFF the inventory is open
		tower_modify_ui->update_inventory(info.get_inventory_state());

	}

	//called by the main frontend class in response to user input. This is where we make the TowerUpgrade UI 
	//the active window
    void handle_tower_upgrade(uint32_t active_tower_ID)
    {
        tower_modify_ui->activate_towerUI(active_tower_ID);
    }

    void display_information(const std::string& base_stats, const std::string& current_stats, const std::string& unit_info);


private:	
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
