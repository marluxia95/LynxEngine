#include "Core/application.h"
#include "luaRuntime.h"

#include "luaAPI_core.h"

namespace Lynx::Lua {

    LuaRuntime::LuaRuntime(Scene* scene) : ECS::System(scene) 
    {
        

    }

    LuaRuntime::~LuaRuntime()
    {
        lua_close(state);
    }

    void LuaRuntime::Init()
    {
        state = luaL_newstate();
        luaL_openlibs(state);
        {
            luaL_requiref(state, "LynxCore", luaopen_lynxcoreapi, 1);
            lua_pop(state, 1);
        }

        log_debug("Lua initialized sucessfully");
    }

    void LuaRuntime::Update()
    {


    }

    void LuaRuntime::lua_error(const char* file)
    {
        log_error("LUA Error in %s : %s", file, lua_tostring(state,-1));
    }

    void LuaRuntime::OnEntityAdded(Entity ent)
    {
        auto lua_component = scene->GetComponent<LuaScript>(ent);
        const char* file_path = lua_component->path;
        
        int success;

        success = luaL_loadfile(state, file_path);

        if(success != LUA_OK) {
            lua_error(file_path);
            return;
        }

        log_debug("LUA : Running file %s", file_path);

        success = lua_pcall(state, 0, LUA_MULTRET, 0);

        if(success != LUA_OK) {
            lua_error(file_path);
            return;
        }

        log_debug("LUA After run file");
    }

    void LuaRuntime::OnEntityRemoved(Entity ent)
    {

    }

    
}