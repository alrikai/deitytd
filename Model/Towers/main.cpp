#include "Essences.hpp"
#include "EssenceSynthesis.hpp"
#include "TowerDispatcher.hpp"

#include "Tower.hpp"

#include <memory>
#include <iostream>

int main()
{
    tower_properties props;
    std::unique_ptr<aphrodite> aph = std::unique_ptr<aphrodite> (new aphrodite);
    std::unique_ptr<apollo>    apo = std::unique_ptr<apollo> (new apollo);
    props = combine_static_essences(aph.get(), apo.get());
    std::cout << "essence synthesis: " << props << std::endl;

    std::unique_ptr<ares>      are = std::unique_ptr<ares> (new ares);
    std::unique_ptr<artemis>   art = std::unique_ptr<artemis> (new artemis);
    props = combine_static_essences(aph.get(), apo.get(), are.get(), art.get());
    std::cout << "non-sessence synthesis: " << props << std::endl;

/*
 *  test the dynamic multiple dispatch
 */
    //generate the type-erased objects
    std::unique_ptr<essence> r_aph = std::unique_ptr<aphrodite> (new aphrodite);
    std::unique_ptr<essence> r_apo = std::unique_ptr<apollo> (new apollo);
    std::unique_ptr<essence> r_are = std::unique_ptr<ares> (new ares);
    std::unique_ptr<essence> r_ath = std::unique_ptr<athena> (new athena);
    std::unique_ptr<essence> r_art = std::unique_ptr<artemis> (new artemis);
    std::unique_ptr<essence> r_dem = std::unique_ptr<demeter> (new demeter);

    tower_generator tower_gen;
    props = tower_gen.combine(r_aph.get(), r_apo.get());
    const float row_pos = 0.0f;
    const float col_pos = 0.0f;

    std::string tower_id {"tower_"};
	props = tower_gen.combine(r_ath.get(), r_art.get());
    const std::string ta_id = tower_id + "ath_art";
    Tower ta(std::move(props), ta_id, 6, row_pos, col_pos);
	std::cout << ta << std::endl;
    
	props = tower_gen.combine(r_aph.get(), r_apo.get(), r_are.get());
    props = tower_gen.combine(r_ath.get(), r_art.get(), r_dem.get());
    props = tower_gen.combine(r_ath.get(), r_ath.get(), r_dem.get());
    const std::string tz_id = tower_id + "apo_art_ath";
    Tower tz(std::move(props), tz_id, 3, row_pos, col_pos);
	std::cout << tz << std::endl;

    return 0;
}

