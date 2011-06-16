SDL = {
	basepath = PathDir(ModuleFilename()),

	OptFind = function (name, required)
		local check = function(option, settings)
			option.value = false
			option.use_sdlconfig = false
			option.use_win32sdl = false
			option.use_osxframework = false
			option.lib_path = nil
			
			if ExecuteSilent("sdl-config") > 0 and ExecuteSilent("sdl-config --cflags") == 0 then
				option.value = true
				option.use_sdlconfig = true
			end
			
			if platform == "win32" then
				option.value = true
				option.use_win32sdl = true
			end
			
			if platform == "macosx" then
				option.value = true
				option.use_osxframework = true
				option.use_sdlconfig = false
			end
		end
		
		local apply = function(option, settings)
			if option.use_sdlconfig == true then
				settings.cc.flags:Add("`sdl-config --cflags`")
				settings.link.flags:Add("`sdl-config --libs`")
			end

			if option.use_osxframework == true then
				client_settings.link.frameworks:Add("SDL")
				client_settings.cc.includes:Add("/Library/Frameworks/SDL.framework/Headers")
			end

			if option.use_win32sdl == true then
				settings.cc.includes:Add(SDL.basepath .. "/include")
				settings.link.libpath:Add(SDL.basepath .. "/vc2005libs")
				settings.link.libs:Add("SDL")
				settings.link.libs:Add("SDLmain")
			end
		end
		
		local save = function(option, output)
			output:option(option, "value")
			output:option(option, "use_sdlconfig")
			output:option(option, "use_win32sdl")
			output:option(option, "use_osxframework")
		end
		
		local display = function(option)
			if option.value == true then
				if option.use_sdlconfig == true then return "using sdl-config" end
				if option.use_win32sdl == true then return "using supplied win32 libraries" end
				if option.use_osxframework == true then return "using osx framework" end
				return "using unknown method"
			else
				if option.required then
					return "not found (required)"
				else
					return "not found (optional)"
				end
			end
		end
		
		local o = MakeOption(name, 0, check, save, display)
		o.Apply = apply
		o.include_path = nil
		o.lib_path = nil
		o.required = required
		return o
	end
}
