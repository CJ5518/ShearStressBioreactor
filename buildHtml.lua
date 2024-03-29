--Builds html.hpp
--Because writing this script was legitimately much easier than figuring out SPIFFS

--Usage: lua buildHtml.lua


local isWindows = package.config:sub(1,1) == "\\";

local getListOfFiles;

--If you're not on windows I believe in your ability to change the system calls
if isWindows then
	getListOfFiles = function()
		os.execute("dir data /B > delet.txt");
		local file = io.open("delet.txt", "r");
		local list = {};
		for line in file:lines() do
			list[#list+1] = line;
		end
		file:close();
		os.execute("del delet.txt");
		return list;
	end
end

local filePattern = [[
#ifndef HTML_HPP
#define HTML_HPP

//This file contains various files for the web server, not just html but also css and js
//Doing this was so much easier than SPIFFS what can I say

#include <Arduino.h>

%s

#endif

]]

local list = getListOfFiles();

local stringList = {};

for i, v in ipairs(list) do
	--Read file content
	local file = io.open("data/" .. v, "r");
	local content = file:read("*a");
	file:close();
	--Remove whitespace
	content = content:gsub("\\", "\\\\")
	content = content:gsub("\"", "\\\"");
	content = content:gsub("\n", "\\n");
	content = content:gsub("\t", "\\t");
	--const char index_html[] PROGMEM = "....";
	
	local filename, extension = v:match("(%w+)%.(%w+)");
	local content = string.format('const char %s_%s[] PROGMEM = "%s";', filename, extension, content);
	stringList[#stringList+1] = content;


	--Go ahead and print the server stuff out too
--server->on("/", HTTP_ANY, [](AsyncWebServerRequest *request){
--request->send_P(200, "text/html", index_html);
--});
	if filename == "index" then
		print('server->on("/", HTTP_ANY, [](AsyncWebServerRequest *request) {');
	else
		print('server->on("/' .. v .. '", HTTP_ANY, [](AsyncWebServerRequest *request) {');
	end
	local extensionMIME = extension;
	if extension == "js" then extensionMIME = "javascript" end
	print(string.format('    request->send_P(200, "text/%s", %s_%s);', extensionMIME, filename, extension));
	print('});');
end

local finalString = filePattern:format(table.concat(stringList, "\n"));

local file = io.open("include/html.hpp", "w");
file:write(finalString);
file:close();
