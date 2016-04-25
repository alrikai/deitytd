/* TowerUI.cpp -- part of the DietyTD Views subsystem implementation 
 *
 * Copyright (C) 2015 Alrik Firl 
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef TD_VIEWS_UI_TOWER_UI_HPP
#define TD_VIEWS_UI_TOWER_UI_HPP

#include <memory>
#include <iostream>

#include <CEGUI/CEGUI.h>
#include <CEGUI/RendererModules/Ogre/Renderer.h>

#include "shared/Player.hpp"
#include "shared/PlayerInventory.hpp"
#include "shared/common_information.hpp"



struct TowerUpgradeUI
{
	TowerUpgradeUI(CEGUI::Window* gui_window)
		: gui_window(gui_window), shared_gamestate_info(nullptr), activetower_ID(-1)
	{
		gui_window->getChild("TowerUpgradeWindow")->getChild("OpenWordCombineButton")->subscribeEvent(CEGUI::PushButton::EventClicked, 
			CEGUI::Event::Subscriber(&TowerUpgradeUI::word_combination_evthandler, this));
        //this will be invisible by default, and it'll become visible when a tower is clicked
        gui_window->getChild("TowerUpgradeWindow")->setVisible(false);

        initialize_wordcomboUI();
	}

	void initialize_wordcomboUI();
    void activate_towerUI(uint32_t active_tower_ID);

	void update_inventory(const PlayerInventory* inventory);
    inline void register_shared_towerinfo(std::shared_ptr<GameInformation<CommonTowerInformation, TDPlayerInformation>> shared_info)
	{
        shared_gamestate_info = shared_info;
	}


	//need to have the various button handlers here
	bool wordcombine_combinebtn(const CEGUI::EventArgs &e);
	bool wordcombine_previewbtn(const CEGUI::EventArgs &e);
	bool wordcombine_clearbtn(const CEGUI::EventArgs &e);
    bool wordcombine_cancelbtn(const CEGUI::EventArgs &e);
    
	bool word_combination_evthandler(const CEGUI::EventArgs &e);
    
    bool handle_inventory_item_dropped(const CEGUI::EventArgs& args);


	CEGUI::Window* gui_window;
	CEGUI::Window* gui_wordcombine_window;
    //std::vector<CEGUI::Window*> session_word_slots;
    std::shared_ptr<GameInformation<CommonTowerInformation, TDPlayerInformation>> shared_gamestate_info;
    CEGUI::HorizontalLayoutContainer* wordslot_layout;
    //std::shared_ptr<PlayerInventory> inventory;

    uint32_t activetower_ID;
};


#endif
