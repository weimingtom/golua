
#include <lua.h>
#include <lauxlib.h>
#include <string.h>
#include <cothread.h>
 #include <unistd.h>
#include "_cgo_export.h"

static void ll_handler(lua_State *L, lua_Debug *ar){
	// lua_getinfo(L, "Snl", ar);
 	// printf("Line hook %s %s %d\n", ar->source, ar->name, ar->currentline);
	lua_yield(L, 0);
}

int kickoff(co_thread * ct) {
	int ret = lua_resume(ct->s, NULL, ct->nargs);
	if( ret > 1) {
		luaL_error(ct->p, "thread error: %s", lua_tostring(ct->p, -1));
	}
	return ret;
}

int resume(co_thread * ct) {
	int ret = lua_status(ct->s);
	if (ret == 1) {
		ret = lua_resume(ct->s, NULL, 0);
		if( ret > 1) {
			luaL_error(ct->p, "thread error: %s", lua_tostring(ct->p, -1));
		}
	}
	return ret;
}

int ct_sleep(lua_State *L) {
	int n = luaL_checkinteger(L, 1);
	usleep(n * 1000);
	return 0;
}

void register_sleep(lua_State *L, char* mod) {
	lua_getglobal(L, mod);
	lua_pushcfunction(L, ct_sleep);
	lua_setfield(L, -2, "sleep");
}

co_thread ll_cothread(lua_State *L) {
	if (lua_isfunction(L, 1) == 0) {
		luaL_error(L, "It should be a lua function");
	}
	int n = lua_gettop(L);

	lua_State *T = lua_newthread(L);
	lua_pushthread(T);
	lua_rawsetp(L, LUA_REGISTRYINDEX, (void *)T);

	if (T == NULL) {
		luaL_error(L, "unable to create new state");
	}
	lua_insert(L, 1);

	lua_sethook(T, ll_handler,  LUA_MASKCOUNT, 7);

	lua_xmove(L, T, n);
	co_thread ct = {T, L, 0};
	return ct;
}