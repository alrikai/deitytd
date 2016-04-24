#ifndef TD_PLAYER_INVENTORY_HPP
#define TD_PLAYER_INVENTORY_HPP

#include <array>

/* This should be the graphical component of the player inventory -- the actual inventory state
 * should be obtained from the player (hence I should have a player entity). In theory if I ever
 * want to do multiplayer, it'll be helpful to have all the player state encapsulated in this manner
 */

struct InventoryMetadata
{

    void swap(InventoryMetadata& meta_lhs, InventoryMetadata meta_rhs) 
    {
     
    }
};

class PlayerInventory
{
public:
    static constexpr int NUM_INVENTORY_ROWS = 5;
    static constexpr int NUM_INVENTORY_COLS = 4;
    static constexpr int NUM_INVENTORY_SLOTS = NUM_INVENTORY_ROWS * NUM_INVENTORY_COLS;

    PlayerInventory()
    {
        for (int idx = 0; idx < NUM_INVENTORY_SLOTS; idx++) {
            inventory_occupied[idx] = false;
        }
    }

    //adds a new item to the next available inventory location.
    //the only case I can think of off hand that it'll return false is if the inventory
    //is already full
    bool add_item(InventoryMetadata data)
    {
        bool item_added = false;
        //update the backing data first...
        for (int idx = 0; idx < NUM_INVENTORY_SLOTS; idx++) {
            if(!inventory_occupied[idx]) {
                inventory_data[idx] = data;
                inventory_occupied[idx] = true;
                item_added = true;
                break;
            }
        }
 
        //... then update the GUI (if the item was added)
        //TODO: update the GUI --> NOTE: this might end up being done elsewhere in the frontend actually...

        return item_added;
    }


    bool swap_item(const int item_aidx, const int item_bidx)
    {
        assert(item_aidx >= 0 && item_aidx < NUM_INVENTORY_SLOTS);
        assert(item_bidx >= 0 && item_bidx < NUM_INVENTORY_SLOTS);

        std::swap(inventory_data[item_aidx], inventory_data[item_bidx]);
    }

private:
    std::array<InventoryMetadata, NUM_INVENTORY_SLOTS> inventory_data;
    std::array<bool, NUM_INVENTORY_SLOTS> inventory_occupied;
};

#endif
