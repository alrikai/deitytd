#include "TowerUI.hpp"

void TowerUpgradeUI::initialize_wordcomboUI()
{
	gui_wordcombine_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("DTDWordCombineUI.layout"); 
    //gui_wordcombine_window->setParent(gui_window);
	gui_wordcombine_window->setVisible(false);

	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CombinedStatsEdit")->setText("");
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CurrentStatsEdit")->setText("");

	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("PreviewButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_previewbtn, this));
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CombineButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_combinebtn, this));
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("ClearButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_clearbtn, this));
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CancelButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_cancelbtn, this));

    //TODO: figure out how the hell to make these things show up. 
    //Alternately, I could have everything in the inventory already have DragContainers w/ StaticImages, but have them be the 
    //default inventory background thumbnail, and have them be 'inactive'. Then when the user gets an item in their inventory, 
    //need to update the thumbnail image to be that item, and 'activate' the dragcontainer + thumbnail group. 
    //... but I still DO need to figure out how to programatically make GUI window hierarchies and have them actually display && work
    //just for testing purposes, add in an item to the inventory
    //auto target_inventory_slot = gui_wordcombine_window->getChild("InventoryWindow")->getChild("slot_0");


    int inventory_rows = 5;
    int inventory_cols = 4;
    int inventory_idx = 0;
    for (int row = 0; row < inventory_rows; row++) {
        for (int col = 0; col < inventory_cols; col++) {
            std::string slot_id = "slot_" + std::to_string(inventory_idx);
            std::cout << "subscribing slot " << slot_id << std::endl;
            auto target_inventory_slot = gui_wordcombine_window->getChild("InventoryWindow")->getChild(slot_id);
            target_inventory_slot->subscribeEvent(CEGUI::Window::EventDragDropItemDropped,
                CEGUI::Event::Subscriber(&TowerUpgradeUI::handle_inventory_item_dropped, this));

			//for testing: enable the slot. Also note that you should have the background be non-draggable, and only enable dragging capabilities if 
			//there's an item in that inventory slot.
			auto slot_dragger = reinterpret_cast<CEGUI::DragContainer*>(target_inventory_slot->getChild("dragger"));
			//by default we don't want to have the backgrounds be draggable
			slot_dragger->setDraggingEnabled(false);

            inventory_idx += 1;
        }
    }
  /* 
    auto inventory_slot_dragged = [](const CEGUI::EventArgs &e)
    {
        std::cout << "NOTE: inventory slot dragging..." << std::endl;
        return true;
    };
    target_inventory_slot->getChild("dragger")->subscribeEvent(CEGUI::DragContainer::EventDragStarted, 
            CEGUI::Event::Subscriber(&inventory_slot_dragged));
            */
/*
    auto inventory_slotdrag = static_cast<CEGUI::DragContainer*>(CEGUI::WindowManager::getSingleton().createWindow("DragContainer", "dragslot_0"));
    auto inventory_sloticon = CEGUI::WindowManager::getSingleton().createWindow("DTDLook/StaticImage", "iconslot_0");
    inventory_slotdrag->addChild(inventory_sloticon);
    inventory_sloticon->setSize(CEGUI::USize(CEGUI::UDim(1, 0), CEGUI::UDim(1, 0)));
    inventory_sloticon->setPosition(CEGUI::UVector2(CEGUI::UDim(0, 0), CEGUI::UDim(0,0)));
    inventory_sloticon->setText("slot 0");
    target_inventory_slot->addChild(inventory_slotdrag);
    inventory_slotdrag->setSize(CEGUI::USize(CEGUI::UDim(0.95, 0), CEGUI::UDim(0.95, 0)));
    inventory_sloticon->setPosition(CEGUI::UVector2(CEGUI::UDim(0.05, 0), CEGUI::UDim(0.05,0)));
*/
    wordslot_layout = nullptr;
}


void TowerUpgradeUI::activate_towerUI(uint32_t active_tID)
{
    activetower_ID = active_tID;
    gui_window->getChild("TowerUpgradeWindow")->setVisible(true);
}


//Q: how best to handle the inventory updating? 
void TowerUpgradeUI::update_inventory(const PlayerInventory* inventory)
{
	//std::cout << "Updating UI..." << std::endl;
	for (int row = 0; row < PlayerInventory::NUM_INVENTORY_ROWS; row++) {
	    for (int col = 0; col < PlayerInventory::NUM_INVENTORY_COLS; col++) {

			if(inventory->inventory_occupied[row*PlayerInventory::NUM_INVENTORY_COLS+col]) {
                //std::cout << "inventory[" << row << "][" << col << "] occupied -- " << inventory->inventory_data[row*PlayerInventory::NUM_INVENTORY_COLS+col].letter << std::endl;
			}
		}
	}

}

bool TowerUpgradeUI::handle_inventory_item_dropped(const CEGUI::EventArgs& args)
{
    const CEGUI::DragDropEventArgs& dd_args = static_cast<const CEGUI::DragDropEventArgs&>(args);    
    std::cout << "@inventory dragged handler -- #child: " << dd_args.window->getChildCount() << std::endl;
    if (!dd_args.window->getChildCount())
    {
        // add dragdrop item as child of target if target has no item already
        dd_args.window->addChild(dd_args.dragDropItem);
        // Now we must reset the item position from it's 'dropped' location,
        // since we're now a child of an entirely different window
        dd_args.dragDropItem->setPosition(
            CEGUI::UVector2(CEGUI::UDim(0.05f, 0),CEGUI::UDim(0.05f, 0)));
    }
    

    return true;
}

bool TowerUpgradeUI::word_combination_evthandler(const CEGUI::EventArgs &e)
{
	auto tinfo = shared_gamestate_info->get_towerinfo(activetower_ID);
	std::string tower_info_str = tinfo.get_tower_info_string();

	this->gui_window->getChild("TowerUpgradeWindow")->setVisible(false);

	//populate the window with the information we currently have
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CurrentStatsEdit")->setText(tower_info_str);
	gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CombinedStatsEdit")->setText(tower_info_str);

	//TODO: load the tower image here as well...
    //...
	
	gui_wordcombine_window->enable();
	gui_wordcombine_window->activate();

	//populate the number of word slots for the menu
	auto word_scrollpanel = static_cast<CEGUI::ScrollablePane*>(gui_wordcombine_window->getChild("DTDWordCombinePanel")->getChild("CombinePanel"));
	const int num_word_slots = tinfo.num_wordslots;

    //NOTE: we want to have ~4 per panel view
	static constexpr int SLOTS_PER_PANEL = 4;
	auto panel_dims = word_scrollpanel->getWidth();
    const float wordslot_width = 400; //panel_dims.d_offset / SLOTS_PER_PANEL;

    //this should have always been destroyed BEFORE we reach here
    if(wordslot_layout) {
       std::cout << "ERROR -- check your logic, wordslot layout should have been destroyed at this point " << __FILE__ << __LINE__ << std::endl; 
    }
    wordslot_layout = static_cast<CEGUI::HorizontalLayoutContainer*>(CEGUI::WindowManager::getSingleton().createWindow("HorizontalLayoutContainer", "WordSlotLayout"));


    float running_slotpos = 0;
	//TODO: need to destroy these widgets when this menu is closed
	for (int slot_idx = 0; slot_idx < num_word_slots + 5; slot_idx++) {

		std::string word_slot_conttext = "Word_" + std::to_string(slot_idx) + "_Container";
		std::string word_slot_btntext = "Word_" + std::to_string(slot_idx) + "_Button";

        //CEGUI::DragContainer* word_dragslot = static_cast<CEGUI::DragContainer*>(CEGUI::WindowManager::getSingleton().createWindow("DragContainer", word_slot_conttext));

		//have the width such that we have 4 per panel view, taking the entire height 
        CEGUI::Window* word_slot_widget = CEGUI::WindowManager::getSingleton().createWindow("DTDLook/Button", word_slot_btntext);
        word_slot_widget->setSize(CEGUI::USize(CEGUI::UDim(0, wordslot_width), CEGUI::UDim(1, 0)));
        word_slot_widget->setPosition(CEGUI::UVector2(CEGUI::UDim(0, running_slotpos), CEGUI::UDim(0,0)));
        running_slotpos += wordslot_width;
		word_slot_widget->setText(word_slot_btntext);
        //word_dragslot->addChild(word_slot_widget);

        wordslot_layout->addChild(word_slot_widget);
        //wordslot_layout->addChild(word_dragslot);
		//session_word_slots.push_back(word_slot_widget);
	}
    //attach the layout to the scrollable panel	
    word_scrollpanel->addChild(wordslot_layout); 
    word_scrollpanel->initialiseComponents();

	gui_wordcombine_window->setVisible(true);
	
    //-------------------
    std::cout << "layout visible: " << wordslot_layout->isVisible() << " size: " << wordslot_layout->getSize() << std::endl; 
    std::cout << "scrollpanel visible: " << word_scrollpanel->isVisible() << " size: " << word_scrollpanel->getSize() << std::endl; 
   
	for (int slot_idx = 0; slot_idx < num_word_slots; slot_idx++) {
        std::string child_id = "Word_" + std::to_string(slot_idx) + "_Button";
        std::cout << "word slot " << slot_idx << " visible: " <<  wordslot_layout->getChild(child_id)->isVisible() << " size: " << wordslot_layout->getChild(child_id)->getSize() << std::endl;
    }
 
    //-------------------

	//NOTE: need to switch back to the regular UI once this one is exitted
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(gui_wordcombine_window);

	//NOTE: also need to spawn the word combination window here...
	return true;
}


bool TowerUpgradeUI::wordcombine_combinebtn(const CEGUI::EventArgs &e)
{

	return true;
}

bool TowerUpgradeUI::wordcombine_previewbtn(const CEGUI::EventArgs &e)
{

	return true;
}

bool TowerUpgradeUI::wordcombine_clearbtn(const CEGUI::EventArgs &e)
{

	return true;
}

bool TowerUpgradeUI::wordcombine_cancelbtn(const CEGUI::EventArgs &e)
{
	gui_wordcombine_window->setVisible(false);

    CEGUI::WindowManager::getSingleton().destroyWindow(wordslot_layout);
    wordslot_layout = nullptr;
/*
    //how to delete GUI widgets?
	for (auto wslot_widgetit : session_word_slots) {
        CEGUI::WindowManager::getSingleton().destroyWindow(wslot_widgetit);
	}
    session_word_slots.clear();
*/  
	//TBH, no idea what the difference between disable and deactivate is here
    gui_wordcombine_window->disable();
    gui_wordcombine_window->deactivate();
    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(gui_window);
	return true;
}
