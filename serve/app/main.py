import logging

from fastapi import FastAPI

import deitytd

app = FastAPI(
    title="Deity TD",
    description="The worst TD you'll never play",
    version="0.1.0"
)

@app.on_event("startup")
def startup():
    logging.warning(f"starting up... {deitytd.__version__}")


@app.on_event("shutdown")
def shutdown():
    logging.warning(f"shutting down... {deitytd.__version__}")

@app.post("/start")
def start_game(seed: int = 1337):
    app.state.gamestate = deitytd.dtdcore.TowerDefense(seed)
    app.state.gamestate.init_game()
    app.state.gamestate.start_game()

@app.post("/stop")
def stop_game():
    app.state.gamestate.stop_game()

@app.get("/state")
def get_state():
    gamestate_snapshot = app.state.gamestate
    #TODO: package into appropriate data format to return with... or just pickle it or something

