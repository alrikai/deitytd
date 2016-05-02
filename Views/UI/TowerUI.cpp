#include "TowerUI.hpp"

void TowerUpgradeUI::initialize_wordcomboUI()
{
	gui_wordcombine_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("DTDWordCombineUI.layout"); 
    //gui_wordcombine_window->setParent(gui_window);
	gui_wordcombine_window->setVisible(false);

	gui_wordcombine_layout = reinterpret_cast<CEGUI::HorizontalLayoutContainer*>(gui_wordcombine_window->getChild("WordCombineBackground"));

	auto word_combine_window = gui_wordcombine_layout->getChild("DTDWordCombinePanel");
	word_combine_window->getChild("CombinedStatsEdit")->setText("");
	word_combine_window->getChild("CurrentStatsEdit")->setText("");

	word_combine_window->getChild("PreviewButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_previewbtn, this));
	word_combine_window->getChild("CombineButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_combinebtn, this));
	word_combine_window->getChild("ClearButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_clearbtn, this));
	word_combine_window->getChild("CancelButton")->subscribeEvent(
			CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber(&TowerUpgradeUI::wordcombine_cancelbtn, this));

        
    gui_inventory_window = CEGUI::WindowManager::getSingleton().loadLayoutFromFile("DTDInventory.layout");
	gui_inventory_window->setVisible(true); //(false);

	//NOTE: you can make this more robust by getting the position and size, then placing the inventory after it.
	//it works as it currently is though
    //auto wcombo_position = gui_wordcombine_layout->getPositionOfChild(word_combine_window);
	//std::cout << "word combine panel position: " << wcombo_position << std::endl;
    gui_wordcombine_layout->addChild(gui_inventory_window);
    gui_inventory_window->setPosition(CEGUI::UVector2(CEGUI::UDim(0.6,0),CEGUI::UDim(0,0)));

    constexpr int inventory_rows = PlayerInventory::NUM_INVENTORY_ROWS;
    constexpr int inventory_cols = PlayerInventory::NUM_INVENTORY_COLS;
    int inventory_idx = 0;
    for (int row = 0; row < inventory_rows; row++) {
        for (int col = 0; col < inventory_cols; col++) {
            std::string slot_id = "slot_" + std::to_string(inventory_idx);
            std::cout << "subscribing slot " << slot_id << std::endl;
            auto target_inventory_slot = gui_inventory_window->getChild("InventoryPanel")->getChild(slot_id);

			//for testing: enable the slot. Also note that you should have the background be non-draggable, and only enable dragging capabilities if 
			//there's an item in that inventory slot.
			auto slot_dragger = reinterpret_cast<CEGUI::DragContainer*>(target_inventory_slot->getChild("dragger"));
            slot_dragger->subscribeEvent(CEGUI::Window::EventDragDropItemDropped,
                CEGUI::Event::Subscriber(&TowerUpgradeUI::handle_inventory_item_dropped, this));

			//by default we don't want to have the backgrounds be draggable
			slot_dragger->setDraggingEnabled(false);

            inventory_idx += 1;
        }
    }

	//load the imageset for handling the letter tiles -- Q: do we even need to do this?
	//CEGUI::ImagesetManager::getSingleton().createImageset("DTDLetters.imageset");
	//letter_imgset = CEGUI::ImagesetManager::getSingleton().getImageset("DTDLetters");

    auto disable_letter_slot = [] (CEGUI::Window* target_letter_slot) {
        //This is to disable the word slot
        //NOTE: just call enable() on it to re-activate the window
        target_letter_slot->disable();
        target_letter_slot->setDragDropTarget(false);
    };

    //Initially, (i.e. when we first make this window), we just want to have EVERYTHING deactivated, 
    //since there will be no towers or anything. (and it wont even be visible)
	//TODO: this is a completely arbitrary # (is just the size of the inventory). Should make this the longest word
	//in the dictionary, and make the inventory larger
    const int max_num_letters = 20;
	auto word_panel = gui_wordcombine_layout->getChild("DTDWordCombinePanel")->getChild("WordDropperPanel");
    for (int letter_slotidx = 0; letter_slotidx < max_num_letters; letter_slotidx++) {
        std::string slot_id = "letter_" + std::to_string(letter_slotidx);
        auto target_letter_slot = word_panel->getChild(slot_id);

        //we never want to be able to drag FROM these slots (I think...)
		auto slot_dragger = reinterpret_cast<CEGUI::DragContainer*>(target_letter_slot->getChild("ldragger"));
        slot_dragger->setDraggingEnabled(false);
        
		slot_dragger->subscribeEvent(CEGUI::Window::EventDragDropItemDropped,
                CEGUI::Event::Subscriber(&TowerUpgradeUI::handle_letter_item_dropped, this));
	
		disable_letter_slot(target_letter_slot);


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
}


void TowerUpgradeUI::activate_towerUI(uint32_t active_tID)
{
    activetower_ID = active_tID;
    gui_window->getChild("TowerUpgradeWindow")->setVisible(true);
}


//Q: how best to handle the inventory updating? We would need to have some sort of 'previous' inventory state, and
//just change parts of it? Or just wholesale re-create the inventory each time?
void TowerUpgradeUI::update_inventory(const PlayerInventory* inventory)
{
	//std::cout << "Updating UI..." << std::endl;
	for (int row = 0; row < PlayerInventory::NUM_INVENTORY_ROWS; row++) {
	    for (int col = 0; col < PlayerInventory::NUM_INVENTORY_COLS; col++) {

            int inventory_idx = row*PlayerInventory::NUM_INVENTORY_COLS+col;
			if(inventory->inventory_occupied[inventory_idx]) {
                //std::cout << "inventory[" << row << "][" << col << "] occupied -- " << inventory->inventory_data[row*PlayerInventory::NUM_INVENTORY_COLS+col].letter << std::endl;
                
                std::string slot_id = "slot_" + std::to_string(inventory_idx);
                auto target_inventory_slot = gui_inventory_window->getChild("InventoryPanel")->getChild(slot_id);

			    auto slot_dragger = reinterpret_cast<CEGUI::DragContainer*>(target_inventory_slot->getChild("dragger"));
   			    //by default we don't want to have the backgrounds be draggable
			    slot_dragger->setDraggingEnabled(true);

                //Q: can we just set the text? on the staticimage? or do we need to have an imageset of letters?
                auto target_letter = inventory->inventory_data[inventory_idx].letter;
				std::transform(target_letter.begin(), target_letter.end(), target_letter.begin(), ::toupper);
				std::string letter_tilename = "DTDLetters/" + target_letter;

				auto slot_simage = slot_dragger->getChild("image");
                slot_simage->setProperty("Image", letter_tilename); 
			}
		}
	}
}

//TODO: need to figure out a good way to manage the inventory. How to handle swapping items, 
//reorganizing them, and maintaining the state between the player inventory and how the user 
//re-arranges the items in the UI (i.e. how to keep track of both)
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

bool TowerUpgradeUI::handle_letter_item_dropped(const CEGUI::EventArgs& args)
{
	//TODO: this is where you would want to put the tile from the inventory on the tile in the word combination set.
	//Will need to store the state for the inventory tile to be moved, and figure out a good way to restore where 
	//to put the inventory tile is the user clears it. 
    std::cout << "dropped letter onto slot..." << std::endl;
	return true;
}

bool TowerUpgradeUI::word_combination_evthandler(const CEGUI::EventArgs &e)
{
	auto tinfo = shared_gamestate_info->get_towerinfo(activetower_ID);
	std::string tower_info_str = tinfo.get_tower_info_string();

	this->gui_window->getChild("TowerUpgradeWindow")->setVisible(false);

	//populate the window with the information we currently have
	gui_wordcombine_layout->getChild("DTDWordCombinePanel")->getChild("CurrentStatsEdit")->setText(tower_info_str);
	gui_wordcombine_layout->getChild("DTDWordCombinePanel")->getChild("CombinedStatsEdit")->setText(tower_info_str);

	//TODO: load the tower image here as well... this will have to be a dynamic image, as we are 
	//procedurally generating the tower models. Should be something where we take a 2D snapshot
	//of the tower's 3D model when we generate it, and store it in a buffer. Then the first time
	//we display it, we need to make a Ogre/CEGUI texture and write the buffer data to it (and finally,
	//display it when we get to here)
    //...
	
	//activate the word combination window...
	gui_wordcombine_window->enable();
	gui_wordcombine_window->activate();
    //... and activate the player inventory window
    gui_inventory_window->enable();
    gui_inventory_window->activate();

	//populate the number of word slots for the menu
	auto word_panel = gui_wordcombine_layout->getChild("DTDWordCombinePanel")->getChild("WordDropperPanel");
	const int num_word_slots = tinfo.num_wordslots;

	std::cout << "activating the first " << num_word_slots << " word slots" << std::endl;

    //this is to enable the word slot again -- so we can drag letters to enabled slots
    auto enable_letter_slot = [] (CEGUI::Window* target_letter_slot) {
        target_letter_slot->enable();
        target_letter_slot->setDragDropTarget(true);
    };

    //NOTE: we will have ALL N letter slots present in the UI, but only M <= N slots will be active,
    //based on the tower itself (and maybe the player? i.e. upgrade to make higher #slots?)
    for (int letter_slotidx = 0; letter_slotidx < num_word_slots; letter_slotidx++) {
        std::string slot_id = "letter_" + std::to_string(letter_slotidx);
        auto target_letter_slot = word_panel->getChild(slot_id);

        //we never want to be able to drag FROM these slots (I think...)
		//auto slot_dragger = reinterpret_cast<CEGUI::DragContainer*>(target_letter_slot->getChild("ldragger"));
        //slot_dragger->setDraggingEnabled(false);
            
        enable_letter_slot(target_letter_slot);
    }

	//NOTE: we assume that the inventory will have already been updated with the player state, 
	//since that happens on every iteration of the frontend (so we don't need to explicitly populate
	//the player inventory)



#if 0
    //NOTE: we want to have ~4 per panel view
	static constexpr int SLOTS_PER_PANEL = 4;
	auto panel_dims = word_panel->getWidth();
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
	
    //-------------------
    std::cout << "layout visible: " << wordslot_layout->isVisible() << " size: " << wordslot_layout->getSize() << std::endl; 
   
	for (int slot_idx = 0; slot_idx < num_word_slots; slot_idx++) {
        std::string child_id = "Word_" + std::to_string(slot_idx) + "_Button";
        std::cout << "word slot " << slot_idx << " visible: " <<  wordslot_layout->getChild(child_id)->isVisible() << " size: " << wordslot_layout->getChild(child_id)->getSize() << std::endl;
    }
 
    //-------------------
#endif

	gui_wordcombine_window->setVisible(true);
	//NOTE: need to switch back to the regular UI once this one is exitted
	CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(gui_wordcombine_window);

	//NOTE: also need to spawn the word combination window here...
	return true;
}


bool TowerUpgradeUI::wordcombine_combinebtn(const CEGUI::EventArgs &e)
{
	std::cout << "combining words...." << std::endl;
	return true;
}

bool TowerUpgradeUI::wordcombine_previewbtn(const CEGUI::EventArgs &e)
{
	std::cout << "previewing word combination...." << std::endl;
	return true;
}

bool TowerUpgradeUI::wordcombine_clearbtn(const CEGUI::EventArgs &e)
{
	std::cout << "clearing word combination...." << std::endl;
	return true;
}

bool TowerUpgradeUI::wordcombine_cancelbtn(const CEGUI::EventArgs &e)
{
	gui_wordcombine_window->setVisible(false);

    //CEGUI::WindowManager::getSingleton().destroyWindow(wordslot_layout);
    //wordslot_layout = nullptr;
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

	gui_inventory_window->disable();
	gui_inventory_window->deactivate();

    CEGUI::System::getSingleton().getDefaultGUIContext().setRootWindow(gui_window);
	return true;
}
