--[[----------------------------------------------------------------------------

Build.lua

Takes Database.lua and produces text lists and other tools for documentation
and updating. Has to be run under Lightroom to be properly translated,
but is not used by users of the plugin.
 
This file is part of MIDI2LR. Copyright 2015 by Rory Jaffe.

MIDI2LR is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

MIDI2LR is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
MIDI2LR.  If not, see <http://www.gnu.org/licenses/>. 
------------------------------------------------------------------------------]]

local Database     = require 'Database'
local LrPathUtils  = import 'LrPathUtils'       

local menulocation = ""

local datafile = LrPathUtils.child(_PLUGIN.path, 'Commands.md')
local file = assert(io.open(datafile,'w'),LOC("$$$/AgImageIO/Errors/WriteFile=The file could not be written.")..' '..datafile)
file:write([=[<!---
  This file automatically generated by Build.lua. To make persistent
  changes, edit Database.lua, not this file
-->
The tables below list all commands currently available in MIDI2LR for all submenus. The title row in each table corresponds with the title of the submenu in the app. Controls marked *button* are intended to be used with a button or key, and unmarked controls are for faders or encoders.

*Note*: <strong>※</strong> symbol indicates that the Adobe Lightroom API documentation does not include this command and an undocumented command may not always behave as expected. **Use with caution!**

### Submenus (click on a name to jump to that section):
]=])
local commandslisting = {}
local headers = {}
for _,v in ipairs(Database.DataBase) do
  local buildingstring = ""
  if v.Group ~= menulocation then
    menulocation = v.Group
    headers[#headers+1] = menulocation
    buildingstring = "\n[back to top](#top)\n\n| <a id=\""..#headers.."\">"..menulocation.."</a> |  |\n| ---- | ---- |\n"
  end
  local experimental = ""
  if v.Experimental  then 
    experimental = "<strong>\226\128\187</strong>"
  end
  buildingstring = buildingstring.."| "..v.Translation..experimental.." | "..v.Explanation
  if v.Type == 'button' then
    buildingstring = buildingstring.." *button*"
  end
  buildingstring = buildingstring.." Abbreviation: `"..v.Command.."`. |\n"
  commandslisting[#commandslisting+1] = buildingstring
end

file:write("\n")
for i,v in ipairs(headers) do
  file:write("- ["..v.."](#"..i..")\n")
end
file:write("\n")

for _,v in ipairs(commandslisting) do
  file:write(v)
end
file:close()

datafile = LrPathUtils.child(_PLUGIN.path, 'GeneratedFromDatabase-ReadMe.txt')
file = assert(io.open(datafile,'w'),LOC("$$$/AgImageIO/Errors/WriteFile=The file could not be written.")..' '..datafile)
file:write ("Running Build.lua generates files for the wiki: Commands.md. This file needs to replace the current file in the wiki.")
file:close()


