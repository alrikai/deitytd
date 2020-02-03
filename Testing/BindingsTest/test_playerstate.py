import pyDeityTD.deitytd as deitytd

def test_create():
    player_info = deitytd.TDPlayerInformation(10, 20, 30)
    assert player_info.get_num_lives() == 10 
    assert player_info.get_num_essence() == 20 
    assert player_info.get_num_gold() == 30
